# Lab 12: Producer, Consumer and the Other Tools

**Student:** Anastasia Rotaru
**Group:** FAF-242
**Course:** Operating Systems, FCIM / FAF, UTM, 2026-2027
**Environment:** Ubuntu 24.04 (arm64) in a Docker container on macOS (Apple Silicon), 8 CPUs, kernel 7.0.14-linuxkit, gcc 13.3.0. All programs were compiled with `gcc -Wall -pthread -o NAME NAME.c`.

Files: `prodcons.c`, `condvar.c`, `readers.c`, `deadlock.c` (the fixed version).

---

## Part 1: Producer and consumer

```
$ gcc -Wall -pthread -o prodcons prodcons.c
$ time ./prodcons
put 1 (in buffer: 1)
put 2 (in buffer: 2)
put 3 (in buffer: 3)
put 4 (in buffer: 4)
    take 1 (in buffer: 3)
put 5 (in buffer: 4)
    take 2 (in buffer: 3)
put 6 (in buffer: 4)
    take 3 (in buffer: 3)
put 7 (in buffer: 4)
    take 4 (in buffer: 3)
put 8 (in buffer: 4)
    take 5 (in buffer: 3)
    take 6 (in buffer: 2)
    take 7 (in buffer: 1)
    take 8 (in buffer: 0)

real	0m2.421s
```

The producer fills the 4 slots at once. After that, every `take` frees one slot and the producer immediately fills it again. The run takes about 2.4 s: 8 items, each followed by the consumer's 0.3 s pause.

### Observe

1. **What stops the producer at 4.** `sem_wait(&empty)` in `producer()`. `empty` starts at `SIZE` = 4 and counts the free slots. After four `put`s it is 0, so the fifth `sem_wait(&empty)` blocks. The producer can continue only after the consumer takes an item and calls `sem_post(&empty)`.

2. **Who waits at the start.** The **consumer**. It is created first, but `full` starts at 0 (nothing to take), so its `sem_wait(&full)` blocks. It wakes up when the producer puts the first item and calls `sem_post(&full)`.

3. **Mutex locked before `sem_wait(&empty)`.** I swapped the two lines in `producer()`:

   ```c
   pthread_mutex_lock(&lock);
   sem_wait(&empty);               /* wait for a free slot */
   ```

   ```
   $ timeout 5 ./prodcons_swap; echo "exit code: $?"
   put 1 (in buffer: 1)
   put 2 (in buffer: 2)
   put 3 (in buffer: 3)
   put 4 (in buffer: 4)
   exit code: 124
   ```

   The program **deadlocks** when the buffer is full and has to be killed by `timeout` (exit code 124). For the fifth item, the producer first takes the mutex, then sleeps in `sem_wait(&empty)` because there is no free slot, **while still holding the mutex**. The only thread that can free a slot is the consumer, but before taking an item it needs `pthread_mutex_lock(&lock)`, which the producer holds. The producer waits for the consumer and the consumer waits for the producer, so neither can ever continue.

   In this run the consumer did not manage to take even one item. Each time the producer released the mutex, it took it again at once for the next item, before the consumer (just woken up) could get it. Only after the buffer was full did the producer stop, holding the lock. The rule: **never wait on a semaphore while holding the mutex** that the other side needs in order to post it.

---

## Part 2: The condition variable

```
$ gcc -Wall -pthread -o condvar condvar.c
$ ./condvar
B: not ready, I sleep
A: message written
B: I read "hello"
```

### Observe

1. **Which variable remembers that the message is ready.** The integer `ready`, protected by the mutex `lock`. A condition variable has no memory: `pthread_cond_signal` only wakes a thread that is already sleeping, and if nobody is waiting, the signal is lost. This is why the state must be kept in a normal variable, unlike the semaphore in `order.c`, whose value itself remembered the `sem_post`.

2. **`if (!ready)` instead of `while (!ready)`.** Three runs:

   ```
   B: not ready, I sleep
   A: message written
   B: I read "hello"
   ```

   (the same output all three times). Yes, it still works **here**. There is only one writer, it sets `ready = 1` before it signals, and it signals only once. When B wakes up, `ready` is therefore always 1.

3. **Why we still write `while`.** A thread can return from `pthread_cond_wait` while the condition is still false. Two reasons:
   - **Spurious wakeups.** POSIX allows `pthread_cond_wait` to return even though no one called `signal`, for example after an interrupted system call inside the implementation.
   - **Another thread got there first.** With several waiters (or `pthread_cond_broadcast`), between the signal and the moment the woken thread takes the mutex again, another thread can take the mutex and use or change the state, for example set `ready` back to 0.

   With `while`, the thread checks the condition again after every wakeup and goes back to sleep if it is not true. With `if`, it would continue with a false condition.

---

## Part 3: Readers and writers

```
$ gcc -Wall -pthread -o readers readers.c
$ ./readers
writer 1 writes 10 (alone)
reader 1 reads 10 (readers inside: 1)
reader 2 reads 10 (readers inside: 2)
reader 3 reads 10 (readers inside: 3)
writer 2 writes 20 (alone)
```

The output was the same in all three runs I did.

### Observe

1. **The two lines that allow many readers but only one writer:**

   ```c
   if (++readers == 1)  sem_wait(&room);   /* the first reader in locks out the writers */
   if (--readers == 0)  sem_post(&room);   /* the last reader out lets the writers in */
   ```

   Only the **first** reader takes `room`, for the whole group of readers. The readers that come after it do not touch `room`, so they enter at the same time. A writer needs `room` for itself (`sem_wait(&room)`), so it cannot enter while any reader is inside, and nobody can enter while a writer holds it. Only the **last** reader to leave gives `room` back.

2. **Why `readers` needs its own lock (`rlock`).** `++readers` and `--readers` are read-add-write in three steps, exactly like `counter++` in Lab 11, so two readers can interleave them. Examples of what goes wrong without `rlock`:
   - Two readers arrive together, both read `readers = 0` and both write 1. Both think they are the first and both call `sem_wait(&room)`. The second one blocks forever, because the first already took `room`.
   - An update is lost and the count becomes too small. `readers` reaches 0 while a reader is still inside, `room` is posted, and a writer enters while that reader is still reading.
   - The count becomes too large. `readers` never comes back to 0, `room` is never released, and the writers wait forever.

   `rlock` also makes the check `== 1` / `== 0` and the matching `sem_wait`/`sem_post(&room)` one indivisible step.

3. **Readers that keep arriving without a pause: what happens to writer 2.** Writer 2 **waits forever**. `room` is released only when `readers` drops back to 0. If a new reader always arrives before the last one leaves, `readers` never becomes 0, so `sem_post(&room)` is never called and writer 2 stays blocked in `sem_wait(&room)`, even though the program as a whole keeps running. This is **starvation**: a thread that is ready to run but never gets the resource. This solution gives priority to readers. A fair version would stop new readers from entering once a writer is waiting.

---

## Part 4: Build a deadlock, then remove it

### The original version (`two()` locks B, then A)

```
$ gcc -Wall -pthread -o deadlock deadlock.c
$ time (timeout 5 ./deadlock; echo "exit code: $?")
one: I have A, I want B
two: I have B, I want A
exit code: 124

real	0m5.001s
```

The two lines appear, then nothing for 5 seconds, then `timeout` kills the program.

### The fixed version (both threads lock A, then B)

In `two()` I swapped the two `pthread_mutex_lock` lines and the two `pthread_mutex_unlock` lines. The submitted `deadlock.c` is this version.

```
$ gcc -Wall -pthread -o deadlock deadlock.c
$ timeout 5 ./deadlock; echo "exit code: $?"
one: I have A, I want B
one: I have both
two: I have B, I want A
two: I have both
done
exit code: 0
```

*Note:* in the fixed version, the message `two: I have B, I want A` is no longer literally true: at that point `two` holds A and wants B. I kept the original `printf` text so that the output matches the one in the lab sheet.

### Observe

1. **Who holds what, and who waits for what.** `one` holds **A** and waits for **B**. `two` holds **B** and waits for **A**.

   ```
             holds                wants
      one ──────────► A  ◄────────────── two
       │                                  ▲
       │ wants                      holds │
       ▼                                  │
       B ─────────────────────────────────┘

   one → wants B → held by two → wants A → held by one → ...
   ```

   The waits form a **circle**, so no thread can continue, and none of them will ever release what it holds.

2. **Without the two `sleep(1)` lines.** I removed both `sleep(1)` lines from the original version and ran it ten times, each time with `timeout 2`:

   | Run | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
   |---|---|---|---|---|---|---|---|---|---|---|
   | Result | **hang** (124) | done | done | **hang** (124) | done | done | done | done | done | done |

   It no longer always hangs: 2 of the 10 runs hung, 8 finished normally. Over 1000 runs, **332** hung. Without the pause, one thread often takes both locks and finishes before the other thread starts, so the circle does not form. This is **worse, not better**. The bug is still there, but it now appears only sometimes and at random. The program passes most tests, then hangs in front of a user, and the problem is hard to reproduce and debug. The `sleep(1)` made the deadlock certain and therefore easy to see.

3. **Why taking the locks in the same order can never deadlock.** If every thread takes A before B, a thread that holds B already holds A, so no thread can ever hold B while waiting for A, and the circle of waits cannot form.
