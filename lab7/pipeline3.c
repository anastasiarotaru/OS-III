#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* does the same as the shell command:   ls /usr/bin | grep x | wc -l   */

int main(void)
{
    int a[2], b[2];                              /* ls -> grep, grep -> wc */
    if (pipe(a) < 0 || pipe(b) < 0) { perror("pipe"); return 1; }

    if (fork() == 0) {                           /* ls: 1 becomes a[1] */
        close(1);
        dup(a[1]);
        close(a[0]); close(a[1]); close(b[0]); close(b[1]);
        execlp("ls", "ls", "/usr/bin", (char *)NULL);
        perror("exec ls");
        exit(127);
    }

    if (fork() == 0) {                           /* grep: 0 <- a[0], 1 -> b[1] */
        close(0);
        dup(a[0]);
        close(1);
        dup(b[1]);
        close(a[0]); close(a[1]); close(b[0]); close(b[1]);
        execlp("grep", "grep", "x", (char *)NULL);
        perror("exec grep");
        exit(127);
    }

    if (fork() == 0) {                           /* wc: 0 becomes b[0] */
        close(0);
        dup(b[0]);
        close(a[0]); close(a[1]); close(b[0]); close(b[1]);
        execlp("wc", "wc", "-l", (char *)NULL);
        perror("exec wc");
        exit(127);
    }

    close(a[0]); close(a[1]);                    /* the parent uses no end */
    close(b[0]); close(b[1]);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    return 0;
}
