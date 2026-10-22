# Operating Systems – Lab 3: The Command Line (Shell, Pipes and Text Filters)

**Student:** Rotaru Anastasia, FAF-242
**Machine:** Linux virtual machine `LinuxLabs`, user `nastea`

## 1. Goal of the work

The purpose of this laboratory work is to navigate the Bash shell efficiently, predict how the shell expands and executes a command line, redirect input and output streams, and answer questions about data with small text filters joined by pipes. The data set (`access.log`, 500 lines) was generated with the command given in the laboratory sheet, so every number below can be reproduced.

## 2. Part 1 – Move fast, find help

### 2.1 Commands and outputs

```text
$ type cd; type ls; type echo; which ls
cd is a shell builtin
ls is aliased to 'ls --color=auto'
echo is a shell builtin
/usr/bin/ls

$ echo $PATH | tr ':' '\n'
/usr/local/sbin
/usr/local/bin
/usr/sbin
/usr/bin
/sbin
/bin
/usr/games
/usr/local/games
/snap/bin

$ nosuchprog; echo "exit code: $?"
nosuchprog: command not found
exit code: 127
```

`man ls` (search with `/sort`, quit with `q`), `ls --help | head`, `man -k "list directory"` and `help cd` were also used to find documentation without a browser.

### 2.2 Observations

1. **Built-ins vs externals.** `cd` and `echo` are shell built-ins, while `ls` is an external program located in `/usr/bin/ls`. `cd` must be a built-in because a child process cannot change the working directory of its parent, which is the shell.
2. **PATH.** There are 9 directories in `PATH`. `ls` is found in `/usr/bin`.
3. **Exit code.** A command that is not found returns exit code `127`.
4. **Manual.** According to `man ls`, the option that sorts by modification time is `-t`.

## 3. Part 2 – What the shell does to your line

### 3.1 Predictions and outputs

Setup: `mkdir -p ~/os-lab3/play && cd ~/os-lab3/play`, then `touch a.c b.c file1.txt file2.txt notes.txt "alfa beta.txt"`.

| Command | My prediction | Real output | Explanation (if different) |
|---|---|---|---|
| `echo *.c` | a.c b.c | a.c b.c | The shell expands the wildcard before `echo` runs. |
| `echo file?.txt` | file1.txt file2.txt | file1.txt file2.txt | `?` matches exactly one character. |
| `echo [a-b]*` | a.c b.c | a.c alfa beta.txt b.c | `*` matches the rest of the name, so `alfa beta.txt` also matches. |
| `echo *.xyz` | error / nothing | *.xyz | No match exists, so the literal pattern is passed to `echo`. |
| `ls alfa beta.txt` | 'alfa beta.txt' | cannot access 'alfa': No such file or directory (and the same for 'beta.txt') | The unquoted space splits the input into two arguments. |
| `ls "alfa beta.txt"` | 'alfa beta.txt' | 'alfa beta.txt' | Quotes keep the string as one argument. |
| `echo '$HOME' "$HOME" \$HOME` | literal text | `$HOME /home/nastea $HOME` | Single quotes and the backslash prevent expansion, double quotes allow it. |
| `echo {1..5} ; echo $((7*6))` | 1 2 3 4 5 and 42 | 1 2 3 4 5 / 42 | Brace expansion and arithmetic expansion. |
| `touch log{1..3}.txt && ls log*` | log1.txt log2.txt log3.txt | log1.txt log2.txt log3.txt | Brace expansion creates three names before `touch` runs. |

### 3.2 Observations

1. **Globbing.** The shell, not `echo`, expands `*.c` and the other patterns into file names. `echo` only prints the arguments it receives.
2. **Fixing `ls`.** The command fails because the space splits `alfa beta.txt` into two arguments. There are two fixes: quote the name (`ls "alfa beta.txt"`) or escape the space (`ls alfa\ beta.txt`).
3. **Quotes.** Double quotes allow variable and command expansion (`$HOME`, `$(whoami)`), while single quotes take everything literally.

## 4. Part 3 – Redirection, chaining and jobs

### 4.1 Commands and outputs

```text
$ cd ~/os-lab3
$ ls /etc/nope > out.txt 2> err.txt
$ wc -l out.txt ; cat err.txt
0 out.txt
ls: cannot access '/etc/nope': No such file or directory

$ wc -l < access.log
500

$ ls /nope 2> /dev/null || echo "failed with code $?"
failed with code 2

$ ls /etc/nope > all.txt 2>&1 ; ls /etc/nope 2>&1 > all.txt
```

Commands with the lab sheet's expected outputs that I ran in the same session:

```text
$ echo first > notes.md ; echo second >> notes.md ; cat notes.md
first
second

$ mkdir -p tmp && cd tmp && echo "I am in $(pwd)" ; cd ~/os-lab3
I am in /home/nastea/os-lab3/tmp

$ sleep 100 &
$ jobs ; kill %1 ; jobs
[1]+  Running                 sleep 100 &
[1]+  Terminated              sleep 100
```

### 4.2 Observations

1. **Redirection targets.** `err.txt` received the error message (stderr redirected with `2>`), while `out.txt` received the standard output, which was empty.
2. **Order of redirection.** The shell processes redirections from left to right. In `> all.txt 2>&1`, stderr is sent to wherever stdout points at that moment, which is the file, so both streams go to `all.txt`. In `2>&1 > all.txt`, stderr is first sent to the screen (the original stdout), and only then stdout goes to the file, so the error stays on the screen.
3. **Standard input.** `wc -l < access.log` gives `wc` the text through standard input, so it prints only the number. `wc -l access.log` passes the file name as an argument, so `wc` also prints the name.
4. **Operators.** `;` runs the next command regardless of the result of the previous one. `&&` runs the next command only if the previous one succeeded (exit code 0). `||` runs the next command only if the previous one failed (non-zero exit code).
5. **Background jobs.** `sleep 100 &` starts the command in the background, `jobs` lists it, and `kill %1` terminates job number 1.

## 5. Part 4 – Answer questions with one-liners

The log has 500 lines with the format `date time user result page`. Each answer is one command line, saved in `oneliners.sh` with the question as a comment above it. Running `bash oneliners.sh` inside `~/os-lab3` prints all answers in the same order.

| Question | One-liner (summary) | Answer |
|---|---|---|
| Q1. How many requests failed? | `grep -c FAIL access.log` | 100 |
| Q2. How many different pages, and which? | `cut -d' ' -f5 access.log \| sort -u` | 4 pages: /page0, /page1, /page2, /page3 |
| Q3. How many requests per page? | `cut -d' ' -f5 access.log \| sort \| uniq -c \| sort -rn` | 125 requests for each page |
| Q4. Which users have the most failures? | `grep FAIL access.log \| cut -d' ' -f3 \| sort \| uniq -c \| sort -rn \| head -3` | user5 (15), user3 (15), user6 (14) |
| Q5. Requests of user3, and failed ones | `grep -cw user3 access.log ; grep -w user3 access.log \| grep -c FAIL` | 72 requests, 15 failed |
| Q6. Last 3 failed requests (time and user) | `grep FAIL access.log \| tail -3 \| cut -d' ' -f2,3` | 10:10 user0, 10:15 user5, 10:20 user3 |
| Q7. Login shells in /etc/passwd | `cut -d: -f7 /etc/passwd \| sort \| uniq -c \| sort -rn` | /usr/sbin/nologin: 40, /bin/false: 6, /bin/bash: 2, /bin/sync: 1 |
| Q8. Why does `ls -l` count one more line? | `ls /etc \| wc -l ; ls -l /etc \| wc -l` | See explanation below |

**Q8 explanation.** `ls -l` prints a first line `total N`, which gives the total number of blocks used by the listed files. Plain `ls` does not print this line, so the count with `-l` is exactly one higher. The difference is not caused by an extra file.

**Stretch (+1).** `grep -E '10:[0-5]0 user[0-9] FAIL' access.log | wc -l` prints 50, which is the number of failed requests at minutes 00, 10, 20, 30, 40 and 50.

**Check yourself.** The per-page counts add up to 500 (4 × 125), and the per-user failure counts add up to the 100 failures of Q1 (14 × 5 + 15 × 2 = 100).

## 6. Part 5 – Your first script

The file `report.sh` reads the log, checks that it exists, prints the total and the number of failed requests, and then prints the failures for each user from `user0` to `user6`.

```text
$ chmod +x report.sh
$ ./report.sh access.log
500 requests, 100 failed
user0: 14 failures
user1: 14 failures
user2: 14 failures
user3: 15 failures
user4: 14 failures
user5: 15 failures
user6: 14 failures

$ ./report.sh nope.log ; echo "exit code: $?"
no such file: nope.log
exit code: 1
```

### 6.1 Observations

1. **Shebang.** The first line `#!/bin/bash` tells the system to run the script with Bash. Running `./report.sh` before `chmod +x` gives `Permission denied`, because the file is not executable yet.
2. **Pathing.** The script must be called as `./report.sh`. The shell searches only the directories in `$PATH` for a plain `report.sh`, and the current directory is not one of them.
3. **Error handling.** The error message is written to standard error with `>&2`, and the script ends with `exit 1` to tell the caller that it failed.

## 7. Conclusion

In this laboratory work I used the Bash shell on Linux. I saw how the shell expands a command line (globbing, brace expansion, variables, arithmetic) before it runs the command, and how quotes change that expansion. I redirected standard output and standard error, and I chained commands with `;`, `&&` and `||`. Pipelines with `grep`, `cut`, `sort`, `uniq`, `head`, `tail` and `wc` answered all eight questions about the log, and the same checks reproduced the results in the script. Turning the commands into `report.sh` showed how scripts use the shebang, the executable bit, exit codes and error output.
