# Lab 11: The Race and the Semaphore

**Student:** Anastasia Rotaru
**Group:** FAF-242
**Course:** Operating Systems, FCIM / FAF, UTM, 2026-2027
**Environment:** Ubuntu 24.04 (arm64) in a Docker container on macOS (Apple Silicon), 8 CPUs visible to the container, kernel 7.0.14-linuxkit, gcc 13.3.0. All programs were compiled with `gcc -Wall -pthread -o NAME NAME.c` (no `-O2`).

Files: `race.c`, `mutex.c`, `sem.c`, `order.c`, `parking.c`, and the optional `atomic.c`.

---

## Part 1: See the race

```
$ gcc -Wall -pthread -o race race.c
$ ./race
counter = 1113300 (expected 2000000)
$ ./race
counter = 1236701 (expected 2000000)
$ ./race
counter = 1013678 (expected 2000000)
```

About half of the additions are lost, and the result is different every time.

### Observe

1. **The three steps of `counter++`.**
   1. **Load:** read the value of `counter` from memory into a CPU register.
   2. **Add:** add 1 to the register.
   3. **Store:** write the register back to `counter` in memory.

2. **How two threads lose one addition.** Suppose `counter` is 5:

   | Thread A | Thread B | `counter` in memory |
   |---|---|---|
   | load 5 into its register | | 5 |
   | | load 5 into its register | 5 |
   | add 1, register = 6 | | 5 |
   | | add 1, register = 6 | 5 |
   | store 6 | | 6 |
   | | store 6 | 6 |

   Both threads did one addition, but `counter` went from 5 to 6, not 7. B read the old value before A had written the new one, so A's addition was overwritten.

3. **Why the result is different every run.** The two threads run on different cores at the same time, and the moment each thread is started, interrupted or moved is decided by the OS scheduler, together with caches and the other work on the machine. These timings are never exactly the same, so the number of times the two threads collide in the load-add-store window, and the number of lost additions, changes from run to run.

---

## Part 2: Fix it: mutex, then semaphore

### 2.1 Mutex

```
$ gcc -Wall -pthread -o mutex mutex.c
$ ./mutex
counter = 2000000 (expected 2000000)
$ ./mutex
counter = 2000000 (expected 2000000)
$ ./mutex
counter = 2000000 (expected 2000000)
```

### 2.2 Semaphore that starts at 1

```
$ gcc -Wall -pthread -o sem sem.c
$ ./sem
counter = 2000000 (expected 2000000)
$ ./sem
counter = 2000000 (expected 2000000)
$ ./sem
counter = 2000000 (expected 2000000)
```

### 2.3 The price of the lock

```
$ time ./race
counter = 1007224 (expected 2000000)

real	0m0.002s
user	0m0.003s
sys	0m0.000s
$ time ./mutex
counter = 2000000 (expected 2000000)

real	0m0.030s
user	0m0.030s
sys	0m0.028s
$ time ./sem
counter = 2000000 (expected 2000000)

real	0m0.024s
user	0m0.036s
sys	0m0.009s
```

Since the times are very small, I ran each program 5 times (`real`, in seconds):

| Program | Run 1 | Run 2 | Run 3 | Run 4 | Run 5 | Correct? |
|---|---|---|---|---|---|---|
| `race`   | 0.002 | 0.002 | 0.002 | 0.002 | 0.002 | no |
| `mutex`  | 0.028 | 0.026 | 0.033 | 0.032 | 0.031 | yes |
| `sem`    | 0.025 | 0.023 | 0.023 | 0.022 | 0.026 | yes |
| `atomic` (optional) | 0.009 | 0.009 | 0.009 | 0.009 | 0.015 | yes |

On this machine, the locked versions are about 12–15 times slower than `race`. Unlike the reference machine, `sem` came out slightly faster than `mutex` here. The difference is only a few milliseconds and depends on how the C library implements the two locks on this machine. What does not change is that both are much slower than the version without a lock.

### Observe

1. **Is the answer always 2 000 000?** Yes. All three runs of `mutex` and all three runs of `sem` print `counter = 2000000`. Only one thread at a time can be between lock and unlock (or `sem_wait` and `sem_post`), so the load-add-store of one thread can no longer be cut by the other.

2. **`sem_init(&s, 0, 2)`.** I changed the start value to 2 and ran it three times:

   ```
   counter = 1974763 (expected 2000000)
   counter = 1960864 (expected 2000000)
   counter = 1975296 (expected 2000000)
   ```

   The result is wrong again. A start value of 2 means two units are available, so **both** threads can pass `sem_wait` at the same time and be inside the critical section together. The race from Part 1 is back. Fewer additions are lost than in `race` because each thread spends most of its time in `sem_wait`/`sem_post`, so the two threads meet less often exactly at `counter++`. But the result is still not guaranteed. A semaphore protects a critical section only if it starts at 1.

3. **Why the locked version is slower.** Every one of the 2 000 000 additions now also does a lock and an unlock. Each of these is an atomic instruction with a memory barrier, which is much more expensive than a plain addition. The lock variable is shared, so its cache line keeps moving between the two cores. When a thread finds the lock taken, it may have to sleep in the kernel (a `futex` system call), which shows up as `sys` time (0.028 s for `mutex`, against 0.000 s for `race`). Finally, the two threads no longer really work in parallel: only one can be adding at a time.

---

## Part 3: A semaphore as a signal

```
$ gcc -Wall -pthread -o order order.c
$ time ./order
B: waiting for the message...
A: message written
B: I read "hello"

real	0m1.005s
```

B prints its first line at once, then nothing happens for one second (A's `sleep(1)`), then A writes the message and B reads it.

### Observe

1. **Start value 1 (`sem_init(&ready, 0, 1)`).**

   ```
   B: waiting for the message...
   B: I read ""
   A: message written
   ```

   B reads an **empty string**. The semaphore already holds one unit, so B's `sem_wait` takes it and returns immediately, without waiting for A. B reads `message` while A is still sleeping. `message` is a global array, so it starts filled with zeros, which is an empty string. A writes `"hello"` one second later, when B has already finished. The order is no longer guaranteed.

2. **What a semaphore that starts at 0 means.** It means "the event has not happened yet": any thread that calls `sem_wait` blocks until another thread signals with `sem_post` that the event (here, "the message is written") has happened.

---

## Part 4: A semaphore as a counter: the parking lot

```
$ gcc -Wall -pthread -o parking parking.c
$ time ./parking
car 1 enters (inside: 1)
car 2 enters (inside: 2)
car 3 enters (inside: 3)
car 3 leaves (inside: 2)
car 1 leaves (inside: 1)
car 4 enters (inside: 2)
car 5 enters (inside: 3)
car 2 leaves (inside: 2)
car 6 enters (inside: 3)
car 5 leaves (inside: 2)
car 4 leaves (inside: 1)
car 6 leaves (inside: 0)

real	0m2.008s
```

Cars 4, 5 and 6 enter only after another car leaves. The run takes 2 seconds: two "waves" of parking, each 1 second long.

### Observe

1. **The largest value of `inside`.** The largest value is **3**. It is guaranteed by `sem_wait(&places);` in `car()`, together with `sem_init(&places, 0, PLACES);` in `main()`, which sets the semaphore to 3. Each car that enters takes one unit. When the value reaches 0, the fourth car blocks in `sem_wait` until a car leaves and calls `sem_post(&places)`. The mutex does not limit the number of cars. It only keeps `inside++`/`inside--` and the matching `printf` together.

2. **`PLACES` set to 1.**

   ```
   car 1 enters (inside: 1)
   car 1 leaves (inside: 0)
   car 2 enters (inside: 1)
   car 2 leaves (inside: 0)
   car 3 enters (inside: 1)
   car 3 leaves (inside: 0)
   car 4 enters (inside: 1)
   car 4 leaves (inside: 0)
   car 5 enters (inside: 1)
   car 5 leaves (inside: 0)
   car 6 enters (inside: 1)
   car 6 leaves (inside: 0)

   real	0m6.011s
   ```

   With one place, the semaphore can only be 0 or 1, so it becomes a **binary semaphore, which works like a mutex** (the same as `sem.c` in Part 2). Only one car can be inside at a time, the cars go strictly one after another, and the run takes 6 seconds instead of 2.

---

## Optional: no lock at all (`atomic.c`)

```
$ gcc -Wall -pthread -o atomic atomic.c
$ time ./atomic
counter = 2000000 (expected 2000000)

real	0m0.009s
```

`atomic_fetch_add` does the load, add and store as one indivisible CPU instruction, so the result is always correct. It is about 3 times faster than the mutex (0.009 s against about 0.03 s), because there is no lock to take and release and no thread ever sleeps. It still costs more than the wrong `race` version (0.002 s), because the two cores must still pass the cache line of `counter` between them. This works only for a single variable. To protect several operations together you still need a mutex or a semaphore.

---

## Conclusion

A semaphore is a single tool used in three different ways, depending only on its start value:

| Start value | Use | Program |
|---|---|---|
| 1 | lock (mutual exclusion, like a mutex) | `sem.c` |
| 0 | signal (B waits for an event from A) | `order.c` |
| N | counter of N free resources | `parking.c` |

Without any of them, two threads that update the same variable lose work (`race.c`), because `counter++` is three steps, not one.
