# Lab 8 — The Same Calls, Inside Your Own OS (xv6)

**Student:** Rotaru Anastasia, FAF-242
**Course:** Operating Systems, FCIM / FAF, UTM, 2026–2027

Files: `user/run.c`, `user/pingpong.c`, `user/primes.c` (optional), `Makefile` (with the `$U/_run\`, `$U/_pingpong\` and `$U/_primes\` lines in UPROGS).

> All outputs below were produced by building this xv6 tree (with the `Makefile` from this folder) and running it in QEMU 8.2.2 (`riscv64-linux-gnu-gcc` 13.3.0, Ubuntu 24.04). Each variant of `run` was run right after a fresh boot.

---

## Part 1 — run, inside xv6

### `echo` (`make qemu`, then `run`)

```
$ run
parent: my pid is 3
child: my pid is 4
hello from the child
parent: child 4 ended with status 0
$
```

### `ls` (step 1.3: `char *args[] = { "ls", 0 };` and `exec("ls", args);`)

```
$ run
parent: my pid is 3
child: my pid is 4
.              1 1 1024
..             1 1 1024
README         2 2 2441
cat            2 3 35424
echo           2 4 34352
sleep          2 5 34072
args           2 6 34072
mystat         2 7 35112
find           2 8 41608
run            2 9 34656
pingpong       2 10 34688
primes         2 11 35424
forktest       2 12 17328
grep           2 13 42784
init           2 14 34816
kill           2 15 34280
ln             2 16 34088
ls             2 17 41568
mkdir          2 18 34328
rm             2 19 34312
sh             2 20 56176
stressfs       2 21 35208
usertests      2 22 200104
grind          2 23 50552
wc             2 24 36384
zombie         2 25 33696
logstress      2 26 36232
forphan        2 27 35096
dorphan        2 28 34544
sync           2 29 33760
console        3 30 0
parent: child 4 ended with status 0
$
```

The child prints the listing of the root directory: name, type (1 = directory, 2 = file, 3 = device), inode number and size in bytes. `run`, `pingpong` and `primes` are in the list because of the UPROGS lines.

### `nosuchprog` (step 1.4: `exec("nosuchprog", args);`)

```
$ run
parent: my pid is 3
child: my pid is 4
exec failed
parent: child 4 ended with status 1
$
```

The lab sheet shows `child 8` because there the program was run after several other commands in the same session; every new process gets the next PID. After a fresh boot the child is PID 4.

### Observe

1. **The three calls in xv6 and on Linux (Lab 7).**
   In xv6, `run.c` uses `fork()`, `exec("echo", args)` and `wait(&status)`. On Linux (Lab 7) it used `fork()`, `execlp("ls", ...)` and `waitpid(pid, &status, 0)`. The three steps are the same: copy, replace the program, wait. The differences are in the details. `exec` in xv6 takes the program name and an array of arguments, and it does not search `PATH`. `wait` in xv6 takes no PID: it waits for any child and returns that child's PID. Linux's `waitpid` can name a specific child.

2. **Who prints "hello from the child": the code of `run`, or `echo`?**
   `echo`. The child calls `exec`, which replaces its code with the code of `echo`. Everything after that line in `run` is gone, so `run` never prints "hello". `echo` prints the words it was given in `args`.

3. **Why is the status 1 here and 127 on Linux?**
   The number comes from the code, not from the operating system. In xv6, `run.c` calls `exit(1)` when `exec` fails, and `wait` stores that raw value, so the parent prints 1. On Linux, the Lab 7 program calls `exit(127)`, which is the usual shell convention for "command not found", and the parent reads it with `WEXITSTATUS(status)`. xv6 has no `WEXITSTATUS` macro, so the raw value is printed directly.

---

## Part 2 — pingpong

### `pingpong` (`make qemu`, then `pingpong`)

```
$ pingpong
6: received ping
5: received pong
$
```

`pingpong` was run in the same session right after `run` (which used PIDs 3 and 4), so the parent is PID 5 and the child PID 6. "ping" comes first, from the child, as required.

### Observe

1. **Why two pipes, and not one for both directions?**
   A pipe carries data in one direction only. It has a read end and a write end. If the parent and child shared one pipe, the parent could read back the byte it just wrote, and nothing would tell the two messages apart. Two pipes give one channel for each direction: `p2c` for parent to child, and `c2p` for child to parent.

2. **Nothing in the code says "wait until the child has replied". Which call makes the parent wait?**
   `read(c2p[0], &b, 1)` in the parent. A read on a pipe blocks until data arrives or every write end is closed. The parent cannot go on until the child writes the byte back. `wait(0)` comes later, and it only waits for the child to exit.

---

## Optional — primes (not graded)

Output of `primes` (every prime from 2 to 35, one per line, in increasing order):

```
$ primes
prime 2
prime 3
prime 5
prime 7
prime 11
prime 13
prime 17
prime 19
prime 23
prime 29
prime 31
$
```
