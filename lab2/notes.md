# Lab 2: Notes

**Rotaru Anastasia, FAF-242**

## Part 2: Observe

**1. Three programs that xv6 ships with**
`ls` (lists a directory), `cat` (prints the content of a file) and `echo` (prints its arguments).

**2. Which two OS features must exist for a pipe to work?**
Processes: the shell creates two processes (with `fork` and `exec`), one for `ls` and one for `grep`, and they run at the same time. Inter-process communication: the kernel provides `pipe()`, a channel in kernel memory, so the output of the first process can be read by the second even though their memories are isolated.

**3. How does the xv6 shell compare to the Linux shell from Lab 1?**
It has the same basic model (prompt, commands, arguments, pipes, redirection), but it is much smaller: no history, no tab completion and no advanced scripting.

## Part 3: Big idea

`kernel/` is the operating system and runs privileged; `user/` holds ordinary programs that can reach the hardware only by making system calls to the kernel.

## Part 4: What I just did

I wrote `user/sleep.c`, which checks its arguments and calls the `pause` system call. The kernel does the waiting, so the process does not use the CPU while it sleeps.
