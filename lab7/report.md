# Lab 7 — Three Small Programs: Processes, Redirection, a Pipe

**Student:** Rotaru Anastasia, FAF-242
**Course:** Operating Systems, FCIM / FAF, UTM, 2026–2027

Files in this folder: `run.c`, `zombie.c`, `redirect.c`, `pipeline.c`, `pipeline3.c` (optional stage).
Compile each with `gcc -Wall -Wextra -o <name> <name>.c`.

PIDs, timestamps and file listings differ on every machine. The values below are from my own runs; the order of the lines and the answers are what matter.

---

## Part 1 — fork + exec + wait

### Run (`run.c`, `ls -l`)

```
parent: my PID is 1680
child:  my PID is 1681, my parent is 1680
total 108
-rwxr-xr-x 1 root root 16400 ... run
-rw------- 1 root root   912 ... run.c
...
parent: child 1681 ended with status 0
```

### Run with `date` (step 1.3)

```
Thu Oct 22 23:35:09 EEST 2026
parent: child 1725 ended with status 0
```

### Run with `nosuchprog` (step 1.4)

```
parent: my PID is 1731
child:  my PID is 1732, my parent is 1731
exec: No such file or directory
parent: child 1732 ended with status 127
```

### Observe

1. **Why does `perror("exec")` print only with `nosuchprog`?**
   `execlp` replaces the memory of the child with the new program. If that works (`ls`, `date`), the code after `execlp` is gone and never runs. `execlp` returns only when it fails, and then the next line runs. `nosuchprog` does not exist, so `execlp` fails, returns, and `perror` prints the reason.

2. **Which number does the child print as "my parent", and whose PID is it?**
   It prints the PID returned by `getppid()`. That is the PID of the parent process, the one that called `fork()`. In the run above it is `1680`, the same number the parent printed as `my PID`.

3. **What status does the parent see with `ls`, and with `nosuchprog`?**
   With `ls` the status is `0`, because `ls` ran and exited normally. With `nosuchprog` the status is `127`, because the child reached `exit(127)` after `execlp` failed. `WEXITSTATUS(status)` extracts that number from the status value that `waitpid` returns.

---

## Part 2 — A zombie, on purpose

### `zombie.c`

Same as `run.c`, with `sleep(20);` placed just before `waitpid`.

### `ps` output while the parent sleeps

```
  PID  PPID STAT CMD
 1734     1 S    ./zombie
 1736  1734 Z    [ls] <defunct>
```

### Observe

1. **Which letter does `ls` have in the STAT column, and what does it mean?**
   `Z`. It means zombie. The child has finished, but its exit status is still stored in the process table because the parent has not called `wait` or `waitpid` yet. The kernel keeps only this small entry.

2. **After 20 seconds, is the `[ls] <defunct>` line still there? Which line of `zombie.c` made it disappear?**
   No, it is gone. The line `waitpid(pid, &status, 0);` makes the parent collect the child's exit status. After that, the kernel removes the zombie entry. The `sleep(20)` only delayed this moment, so the zombie stayed visible in the meantime.

---

## Part 3 — Redirection: `ls -l > out.txt`

### Run (`redirect.c`)

```
open gave descriptor 1
done: the listing is in out.txt
```

```
$ head -3 out.txt
total 108
-rw-r--r-- 1 root root     0 ... out.txt
-rwxr-xr-x 1 root root 16400 ... redirect
```

### Observe

1. **Which number did `open` give, and why exactly that one?**
   It gave `1`. The child called `close(1)` first, so descriptor 1 (standard output) became free. `open` always returns the smallest free descriptor number, so it returns 1. Now descriptor 1 points to `out.txt`.

2. **`ls` was not changed at all. Why does its listing not appear on the screen?**
   `ls` writes to descriptor 1 as always. Descriptor 1 now points to `out.txt`, not to the terminal, so the listing goes into the file. The terminal only shows what the parent prints, which is `done: ...`. The `open gave descriptor 1` line is printed to `stderr` (descriptor 2), which is still the terminal.

3. **To redirect the errors instead (like `2> err.txt`), which descriptor would you close before `open`?**
   Descriptor `2` (standard error). After `close(2)`, the next `open("err.txt", ...)` would return `2`, so errors from `ls` would go to `err.txt`.

---

## Part 4 — A pipe: `ls /etc | wc -l`

### Run (`pipeline.c`) and shell

```
$ ./pipeline
165
$ ls /etc | wc -l
165
```

Both numbers match. The numbers depend on the machine, but on the same machine they must be equal.

### Break it on purpose (`close(p[1])` commented out in the parent)

The program does not end. Stopped with Ctrl-C.

### Observe

1. **Did `./pipeline` and `ls /etc | wc -l` give the same number?**
   Yes, they gave the same number (165 in my run).

2. **After the change, which of the two children never ends, `ls` or `wc`?**
   `wc` never ends. `ls` finishes, because it writes its output and exits normally. `wc` keeps waiting.

3. **Why, in one sentence: what is `wc` waiting for?**
   `wc` waits for end-of-file on its input, which only comes when every copy of the write end `p[1]` is closed; the parent still holds one open because the line that closes it was commented out.

---

## Optional — three stages (`pipeline3.c`)

```
$ ./pipeline3
229
$ ls /usr/bin | grep x | wc -l
229
```

`pipeline3.c` has two pipes and three children. Each child closes all four pipe ends after its `dup`, and the parent does the same before it waits three times. The count matches the shell.
