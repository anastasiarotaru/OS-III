/* fire4.c - Part 2: same race as fire.c, but with FOUR threads.
 * Build: gcc -Wall -Wextra -pthread -o fire4 fire4.c    Run: ./fire4
 */
#include <stdio.h>
#include <pthread.h>

#define N 1000000
#define T 4                    /* number of threads */
long counter = 0;              /* one variable, shared by all threads */

void *work(void *arg) {
    long id = (long)arg;
    printf("thread %ld sees counter = %ld\n", id, counter);
    for (int i = 0; i < N; i++)
        counter++;             /* NOT atomic: read, add, write */
    return NULL;
}

int main(void) {
    pthread_t t[T];
    for (long i = 0; i < T; i++)
        pthread_create(&t[i], NULL, work, (void *)(i + 1));
    for (int i = 0; i < T; i++)
        pthread_join(t[i], NULL);
    printf("expected: %d\n", T * N);
    printf("counter:  %ld   (lost: %ld)\n", counter, (long)T * N - counter);
    return 0;
}
