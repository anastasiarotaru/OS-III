# Lab 17: Threads Share Memory, and Race

**Student:** Anastasia Rotaru
**Group:** FAF-242
**Course:** Operating Systems, FCIM / FAF, UTM, 2026-2027
**Environment:** Ubuntu 24.04 (arm64) in a Docker container on macOS (Apple Silicon), 8 CPUs, kernel 7.0.14-linuxkit, gcc 13.3.0. Programs compiled with `gcc -Wall -Wextra -pthread` (no warnings).

Files: `fire.c` (Part 1, two threads) and `fire4.c` (Part 2, the same program with four threads).

---

## Part 1: Shared and racing

```
$ gcc -Wall -Wextra -pthread -o fire fire.c
$ ./fire
thread 1 sees counter = 0
thread 2 sees counter = 0
expected: 2000000
counter:  1021074   (lost updates: the race)
$ ./fire
thread 1 sees counter = 0
thread 2 sees counter = 7169
expected: 2000000
counter:  1028599   (lost updates: the race)
$ ./fire
thread 1 sees counter = 0
thread 2 sees counter = 9997
expected: 2000000
counter:  955613   (lost updates: the race)
$ ./fire
thread 1 sees counter = 0
thread 2 sees counter = 1846
expected: 2000000
counter:  944732   (lost updates: the race)
$ ./fire
thread 1 sees counter = 0
thread 2 sees counter = 12076
expected: 2000000
counter:  1023445   (lost updates: the race)
```

| Run | Thread 2 saw at start | Final counter | Lost updates | Lost % |
|-----|----------------------:|--------------:|-------------:|-------:|
| 1 | 0      | 1 021 074 |   978 926 | 48.9 % |
| 2 | 7 169  | 1 028 599 |   971 401 | 48.6 % |
| 3 | 9 997  |   955 613 | 1 044 387 | 52.2 % |
| 4 | 1 846  |   944 732 | 1 055 268 | 52.8 % |
| 5 | 12 076 | 1 023 445 |   976 555 | 48.8 % |

The result is never 2 000 000 and it changes on every run. About half of the increments are lost.

### Observe 1: Thread 2 prints a non-zero counter at the start. What does that prove?

It proves that the two threads use **the same memory**. `counter` is one global variable at one address in the address space of the process. Thread 1 started first and had already done some thousands of `counter++` before thread 2 reached its `printf`. Thread 2 never touched the counter, but it still sees 7 169 / 9 997 / 12 076, so it is reading the value that thread 1 wrote, right away and with no copying or message passing. If the threads had separate memory (like two processes after `fork()`), thread 2 would always print 0.

(In run 1 thread 2 also printed 0. That only means thread 2 got to its `printf` before thread 1 had started incrementing. The order is decided by the scheduler and is different every time.)

### Observe 2: Why is the final counter less than 2 000 000, and different each run?

`counter++` looks like one operation in C, but the CPU does it in **three steps**:

1. **read**: load `counter` from memory into a register,
2. **add**: add 1 to the register,
3. **write**: store the register back to `counter` in memory.

The two threads run at the same time on different cores (or are switched by the scheduler), and nothing stops one thread from doing its three steps in the middle of the other thread's three steps. Example, starting with `counter = 100`:

| Time | Thread 1 | Thread 2 | counter in memory |
|------|----------|----------|------------------:|
| 1 | read 100 into reg |                   | 100 |
| 2 |                   | read 100 into reg | 100 |
| 3 | add: reg = 101    |                   | 100 |
| 4 |                   | add: reg = 101    | 100 |
| 5 | write 101         |                   | 101 |
| 6 |                   | write 101         | 101 |

Two increments were done, but the counter went up by only one. The write of thread 2 overwrote the write of thread 1, so **one update is lost**. This happens millions of times during the loops, so the final value is far below 2 000 000.

It is different every run because how the threads interleave depends on timing: which core each thread gets, when the scheduler preempts them, cache traffic, other processes on the machine. None of this is the same twice, so the number of collisions (and lost updates) changes every run. This is what a **race condition** is: the result depends on the timing of the threads.

---

## Part 2: Make it worse, then think

`fire4.c` is the same program, but it creates `T = 4` threads in a loop and the expected value is `4 * N = 4 000 000`.

```
$ gcc -Wall -Wextra -pthread -o fire4 fire4.c
$ ./fire4
thread 1 sees counter = 0
thread 4 sees counter = 0
thread 2 sees counter = 0
thread 3 sees counter = 0
expected: 4000000
counter:  1957983   (lost: 2042017)
$ ./fire4
thread 1 sees counter = 0
thread 2 sees counter = 5425
thread 3 sees counter = 18245
thread 4 sees counter = 27816
expected: 4000000
counter:  1169565   (lost: 2830435)
$ ./fire4
thread 1 sees counter = 0
thread 3 sees counter = 7790
thread 2 sees counter = 0
thread 4 sees counter = 22286
expected: 4000000
counter:  1102559   (lost: 2897441)
$ ./fire4
thread 1 sees counter = 0
thread 2 sees counter = 0
thread 3 sees counter = 38074
thread 4 sees counter = 50542
expected: 4000000
counter:  1066010   (lost: 2933990)
$ ./fire4
thread 1 sees counter = 0
thread 2 sees counter = 0
thread 4 sees counter = 19238
thread 3 sees counter = 0
expected: 4000000
counter:  1035147   (lost: 2964853)
```

| Threads | Expected | Final counter (5 runs) | Lost % |
|--------:|---------:|------------------------|-------:|
| 2 | 2 000 000 | 944 732 to 1 028 599   | about 49 to 53 % |
| 4 | 4 000 000 | 1 035 147 to 1 957 983 | about 51 to 74 % |

**Yes, the loss gets bigger.** With 4 threads not only the number of lost updates grows (about 2 to 3 million instead of 1 million) but also the percentage: in most runs about 70 to 74 % of the increments disappear. The final counter with 4 threads is often hardly larger than with 2 threads, even though twice as much work was done. With more threads running at the same time on different cores, there are more threads between "read" and "write" at any moment, so more writes overwrite each other. The threads also start in a random order (3 before 2, 4 before 3), which shows again that the scheduler decides the order.

### Proposed fix (on paper)

The three steps of `counter++` must run as **one indivisible unit**: while one thread is between its read and its write, no other thread may touch `counter`. The section of code that does this is the **critical section**, and it must be run under **mutual exclusion**. One way to do it is with a lock (mutex):

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

for (int i = 0; i < N; i++) {
    pthread_mutex_lock(&lock);    /* only one thread may pass at a time */
    counter++;                    /* read, add, write: no one can interrupt */
    pthread_mutex_unlock(&lock);
}
```

Other options with the same idea: an atomic increment done by the hardware (`__atomic_fetch_add(&counter, 1, __ATOMIC_SEQ_CST)` or C11 `atomic_long`), or letting each thread count in its own local variable and add it to `counter` once at the end, under the lock.

### Observe 3: In one sentence, what single idea would stop the lost updates?

**Mutual exclusion:** make `counter++` a critical section that only one thread can be inside at a time (for example by taking a lock before it and releasing it after), so that read, add and write of one thread can never be interleaved with another thread's.

---

## Extra note: the race also depends on the compiler

Compiled with `-O2` instead of with no optimisation, `fire.c` printed exactly `2000000`:

```
$ gcc -O2 -pthread -o fire_O2 fire.c && ./fire_O2
thread 1 sees counter = 0
thread 2 sees counter = 0
expected: 2000000
counter:  2000000   (lost updates: the race)
```

This does **not** mean the program is correct. The optimiser saw that the loop only increments `counter`, so it replaced the million `counter++` with a single `counter += 1000000`. The race window became so small that the threads did not hit it in this run, but the code is still not protected. A race that sometimes "works" is still a bug, and it is more dangerous because it is harder to see. Only mutual exclusion makes it correct.
