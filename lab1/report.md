# Lab 7 — Three Small Programs: Processes, Redirection, a Pipe

## Part 1 — fork + exec + wait
1. A successful `execlp` replaces the child's program, so the code after it no
   longer exists in the process. `perror("exec")` can only run when exec fails
   and returns — which happens with `nosuchprog` (ENOENT), followed by `exit(127)`.
2. The child prints its parent's PID (`getppid()`): the same number the parent
   printed for itself (e.g. parent 8076 / child 8077).
3. With `ls` the parent sees status 0; with `nosuchprog` it sees 127 — the value
   passed to `exit(127)` after the failed exec (the shell's "command not found"
   convention).

## Part 2 — A zombie, on purpose
1. `ls` shows `Z` — zombie: the child has exited but its status has not been
   collected; it appears as `[ls] <defunct>`.
2. After 20 seconds the defunct line is gone. The line that removed it is
   `waitpid(pid, &status, 0);` — wait collects the status and frees the
   process-table entry.

## Part 3 — Redirection
1. `open` returned descriptor 1, because descriptors are allocated as the
   smallest free number and the child had just closed 1.
2. `ls` still writes to descriptor 1, unchanged — but in this child, 1 now
   names `out.txt`. Redirection lives in the descriptor table, not in the program.
3. For `2> err.txt` you would `close(2)` before the `open`.

## Part 4 — A pipe
1. Yes: `./pipeline` printed 237 and `ls /etc | wc -l` printed 237.
2. `wc` never ends (ps shows `wc -l` still alive after `ls` finished).
3. `wc` waits for EOF, which arrives only when ALL write ends of the pipe are
   closed — and the parent still holds `p[1]` open.

Optional: `pipeline3.c` (`ls /usr/bin | grep x | wc -l`, two pipes, three
children) prints 225 — identical to the shell.
