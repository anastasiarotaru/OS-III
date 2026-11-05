# Lab 9: Watching System Calls with strace

**Student:** Anastasia Rotaru
**Group:** FAF-242
**Course:** Operating Systems, FCIM / FAF, UTM, 2026-2027
**Environment:** Ubuntu 24.04 (arm64) running in a Docker container on macOS (Apple Silicon). Kernel reported by Docker Desktop: 7.0.14-linuxkit. strace version 6.8, gcc 13.3.0.

---

## Part 1: System calls of `ls -l`

**Command:**

```
strace -f -e trace=openat,read,write,close ls -l
```

**Relevant output:**

```
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libselinux.so.1", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/aarch64-linux-gnu/libpcre2-8.so.0", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/proc/filesystems", O_RDONLY|O_CLOEXEC) = 3
read(3, "nodev\tsysfs\nnodev\ttmpfs\nnodev\tpr"..., 1024) = 566
read(3, "", 1024)                       = 0
openat(AT_FDCWD, ".", O_RDONLY|O_NONBLOCK|O_CLOEXEC|O_DIRECTORY) = 3
write(1, "total 0\n", 8total 0
)                = 8
close(1)                                = 0
close(2)                                = 0
+++ exited with 0 +++
```

**Question:** Which system call does `ls` use to print to the screen? To which descriptor does it write?

**Answer:** `ls` prints its output with the `write` system call, to file descriptor **1** (standard output). In the trace, `write(1, "total 0\n", 8)` is the line that puts the listing on the screen. The directory was empty, so the listing is only the `total 0` line.

**Observation:** `ls` opens the current directory (`openat(..., ".", ...)`) and reads the directory entries with `getdents64`, which is not in the filter, so it does not appear here. The `read` calls visible above come from reading `/proc/filesystems`, which the C library does while starting up.

---

## Part 2: A program that calls the kernel directly

**Program:** `syscalls.c`, which opens `note.txt`, writes `"hello, kernel\n"` to it, closes it, then reopens it and reads it back.

**Build and run:**

```
gcc -Wall -Wextra -o syscalls syscalls.c
./syscalls
```

**Program output (input: none; the program takes no command-line arguments):**

```
open returned descriptor 3
write sent 14 bytes
read got 14 bytes: hello, kernel
```

**Trace command:**

```
strace -e trace=openat,write,read,close ./syscalls
```

**Relevant trace lines and how they map to the C code:**

| C call in `syscalls.c` | System call in the trace | Result |
|---|---|---|
| `open("note.txt", O_CREAT \| O_WRONLY \| O_TRUNC, 0644)` | `openat(AT_FDCWD, "note.txt", O_WRONLY\|O_CREAT\|O_TRUNC, 0644) = 3` | descriptor 3 |
| `write(fd, msg, strlen(msg))` | `write(3, "hello, kernel\n", 14) = 14` | 14 bytes written |
| `close(fd)` | `close(3) = 0` | success |
| `open("note.txt", O_RDONLY)` | `openat(AT_FDCWD, "note.txt", O_RDONLY) = 3` | descriptor 3 |
| `read(fd, buf, sizeof(buf) - 1)` | `read(3, "hello, kernel\n", 63) = 14` | 14 bytes read |
| `printf(...)` | `write(1, "open returned descriptor 3\n", 27) = 27` (and similar) | printed to stdout |

**Question:** The descriptor returned by the first `openat` is 3, not 0, 1 or 2. Why are those three already taken?

**Answer:** Descriptors **0** (stdin), **1** (stdout) and **2** (stderr) are opened by the operating system for every process before it starts running. The first file the program opens therefore gets the lowest free number, which is 3. This is why the `printf` output goes to `write(1, ...)` and not to the file.

**Note on `read`:** The program asks for at most 63 bytes (`sizeof(buf) - 1`), but the file contains only 14, so `read` returns 14. The kernel never returns more data than the file holds.
