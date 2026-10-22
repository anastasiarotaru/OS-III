# Lab 8 — The Same Calls, Inside Your Own OS (xv6)

**Student:** Rotaru Anastasia, FAF-242
**Course:** Operating Systems, FCIM / FAF, UTM, 2026–2027

Files: `user/run.c`, `user/pingpong.c`, `user/primes.c` (optional), `Makefile` (with the `$U/_run\`, `$U/_pingpong\` and `$U/_primes\` lines in UPROGS).

> The outputs below are the **expected outputs from the lab sheet**. They were not produced by running xv6 on my machine. Before committing, replace them with the output you see in QEMU. PIDs and the `ls` listing will be different on your machine.

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
README
cat
echo
sleep
args
mystat
find
run
pingpong
primes
forktest
grep
init
kill
ln
ls
mkdir
rm
sh
stressfs
usertests
grind
wc
zombie
logstress
forphan
dorphan
sync
console

<!-- Reconstructed from the file names in the Makefile (UPROGS) and mkfs, not copied from a real run. Replace with the actual ls output when you run xv6. -->
parent: child 4 ended with status 0
$
```

### `nosuchprog` (step 1.4: `exec("nosuchprog", args);`)

```
$ run
parent: my pid is 3
child: my pid is 8
exec failed
parent: child 8 ended with status 1
$
```

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
10: received ping
9: received pong
$
```

PIDs will differ on your machine. "ping" must come first, from the child.

### Observe

1. **Why two pipes, and not one for both directions?**
   A pipe carries data in one direction only. It has a read end and a write end. If the parent and child shared one pipe, the parent could read back the byte it just wrote, and nothing would tell the two messages apart. Two pipes give one channel for each direction: `p2c` for parent to child, and `c2p` for child to parent.

2. **Nothing in the code says "wait until the child has replied". Which call makes the parent wait?**
   `read(c2p[0], &b, 1)` in the parent. A read on a pipe blocks until data arrives or every write end is closed. The parent cannot go on until the child writes the byte back. `wait(0)` comes later, and it only waits for the child to exit.

---

## Optional — primes (not graded)

Expected output of `primes` (every prime from 2 to 35, one per line, in increasing order):

```
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
```
