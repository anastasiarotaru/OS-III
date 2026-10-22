# Operating Systems – Lab 4: The xv6 Shell from the Outside

**Student:** Rotaru Anastasia, FAF-242
**Machines:** Linux virtual machine `LinuxLabs` (bash) and xv6-riscv in QEMU (xv6 shell)

## Part 1 – Fix sleep for the current xv6

`grep -n "pause\|sleep" user/user.h` shows `25:int pause(int);`, so the system call is now `pause()`. `user/sleep.c` was changed to call `pause(atoi(argv[1]))`, and `$U/_sleep\` is in UPROGS.

```text
$ sleep 20
$ sleep
usage: sleep <ticks>
```

## Part 2 – bash versus the xv6 shell

Each command was run in bash, inside an empty folder `~/shtest` that contains the file `README`, and then in the xv6 shell.

| Command | Tests | bash | xv6 |
|---|---|---|---|
| `echo hello > f; cat < f` | redirection | Prints `hello`. Redirects descriptors 1 and 0 correctly. | Prints `hello`. Redirects files through `open` and `dup`. |
| `ls \| grep READ` | pipe | Prints `README`. stdout is connected to stdin through `pipe`. | Prints `README`. Anonymous pipes work. |
| `(echo a; echo b) \| wc` | grouping, list | Prints `2 2 4` (lines, words, bytes). | Prints `2 2 4`. The grouped subshell is evaluated. |
| `echo one ; echo two` | sequence | Prints `one`, then `two`. | Prints `one`, then `two`. The `;` operator works. |
| `echo bg &` | background | Prints a job line with a PID (for example `[1] 11330`), then `bg`. The PID changes on each run. | Prints `bg` without blocking the prompt. |
| `ls *` | globbing | Expands `*` to the files in the folder (`README f`). | `ls: cannot open *`. The `*` is treated as a literal file name. |
| `echo $HOME` | variables | Prints `/home/nastea`. The variable is replaced. | Prints the literal string `$HOME`. There is no variable support. |
| `echo "a b"` | quotes | Prints `a b`. The shell removes the quotes. | Prints `"a b"` with the quotes included. |
| `mkdir d ; cd d ; ls ..` | PATH | Creates `d`, enters it, and runs `ls ..`. `ls` is found through `$PATH`. | `ls ..` fails with `exec ls failed`, because there is no `PATH`. |
| arrow up, Tab, Ctrl+R | history, completion | Autocompletes names and navigates the history. | Prints raw control characters. There is no line editing. |

### Observations

1. **Features both shells have:** I/O redirection (`<`, `>`), pipes (`|`), sequential lists (`;`), grouping with `( )`, and background execution (`&`). These are implemented with the same system calls (`open`, `dup`, `pipe`, `fork`, `exec`) in both shells. **Features only bash has:** wildcard expansion (globbing `*`), variable substitution (`$`), quote removal, lookup of executables through `PATH`, command history, and tab completion.

2. **`ls` after `cd d` in xv6.** The xv6 shell has no `PATH` variable, so it looks for the executable only in the current directory. Inside `d` there is no `ls` binary. The xv6 programs live in the root directory, so `../ls ..` works:

```text
$ mkdir d
$ cd d
$ ls ..
exec ls failed
$ ../ls ..
```

3. **The most missed feature** in daily work is Tab completion together with command history. From an implementation point of view, the easiest feature to add to `sh.c` would be removing the quote characters while the parameters are processed.

## Part 3 – Who expands `*`? A program that prints its argv

`user/args.c` (for xv6) and `args_linux.c` (for Linux) print `argc` and every `argv[i]`. The xv6 version uses `printf` and `exit(0)`. The Linux version includes `<stdio.h>` and returns `0`. Both are compiled with the same code otherwise.

`$U/_args\` was added to UPROGS in the Makefile.

### Results in bash (Linux)

Run in `~/shtest`, which contains `args` (compiled from `args_linux.c` with `gcc`), `args_linux.c` and `README`:

```text
$ ./args one two three
argc = 4
argv[0] = [./args]
argv[1] = [one]
argv[2] = [two]
argv[3] = [three]

$ ./args *.c "x y"
argc = 3
argv[0] = [./args]
argv[1] = [args_linux.c]
argv[2] = [x y]

$ ./args $HOME
argc = 2
argv[0] = [./args]
argv[1] = [/home/nastea]

$ ./args 'single quoted'
argc = 2
argv[0] = [./args]
argv[1] = [single quoted]
```

### Results in xv6

```text
$ args one     two     three
argc = 4
argv[0] = [args]
argv[1] = [one]
argv[2] = [two]
argv[3] = [three]

$ args *.c "x y"
argc = 4
argv[0] = [args]
argv[1] = [*.c]
argv[2] = ["x]
argv[3] = [y"]

$ args $HOME
argc = 2
argv[0] = [args]
argv[1] = [$HOME]
```

### Observations

1. **Output of `args *.c "x y"` compared.** In bash the program receives `argc = 3`. `*.c` was expanded to `args_linux.c`, and `"x y"` is a single argument `[x y]` with the quotes removed. In xv6 the program receives `argc = 4`. `*.c` is not expanded and arrives as `[*.c]`. The xv6 shell does not interpret quotes, so the space between `x` and `y` splits the string into two arguments: `["x]` and `[y"]`.

2. **Who expands `*.c` and removes the quotes?** The command interpreter (the shell) does this. Neither the kernel nor the executable program does it. The kernel only runs `exec` with the arguments it is given, and the program receives the already processed arguments in `argv`. The xv6 shell has no such logic, so it passes all characters through unchanged.

3. **Multiple spaces.** In both shells a run of spaces acts as a separator, and the extra spaces disappear. `one     two     three` gives three separate arguments in both systems.

**Note.** `args *.c` in bash expands to every `.c` file in the folder. Keep `args.c` (the xv6 source) out of the `~/shtest` folder, otherwise it is also listed.

## Part 4 – Read the shell

```text
$ wc -l user/sh.c
499 user/sh.c

$ grep -n "#define" user/sh.c
8:#define EXEC  1
9:#define REDIR 2
10:#define PIPE  3
11:#define LIST  4
12:#define BACK  5
14:#define MAXARGS 10

$ grep -n "case " user/sh.c
75:  case EXEC:
83:  case REDIR:
93:  case LIST:
101:  case PIPE:
125:  case BACK:
...

$ grep -n "cd" user/sh.c | head
170:        fprintf(2, "cannot cd %s\n", cmd + 3);
```

### Observations

1. **The five kinds of command and the characters that produce them:**
   - `EXEC` (1): a simple command, words separated by spaces.
   - `REDIR` (2): I/O redirection, produced by `<` and `>`.
   - `PIPE` (3): a pipe between two processes, produced by `|`.
   - `LIST` (4): a sequence of commands, produced by `;`.
   - `BACK` (5): background execution, produced by `&`.

2. **Where `cd` is handled and why it is not a separate program.** `cd` is handled directly in the main loop of `main()` (line 170 of `user/sh.c`), which calls the `chdir(cmd + 3)` system call itself. It cannot be an external program, because running an external program requires `fork()` to create a child process. A directory change made in a child process would affect only that child, and the parent shell would stay in the old directory.

3. **Searching for `*` and `$` in `sh.c`.** No line of `sh.c` checks or expands the `*` or `$` characters. This explains the results of Parts 2 and 3: the xv6 shell passes these characters unchanged to the program.

## Conclusion

This laboratory work compared the minimal xv6 shell with GNU bash. The experiments showed a division of responsibilities between the kernel and the interpreter. The kernel provides the primitives (`fork`, `exec`, `pipe`, `chdir`), and the shell parses the command line. Wildcard expansion (`*`), variable substitution (`$`) and quote removal are done by the interpreter before the parameters reach the programs. Reading `sh.c` explained why `cd` must be built into the shell.
