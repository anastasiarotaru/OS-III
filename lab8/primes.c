#include "kernel/types.h"
#include "user/user.h"

// prints the primes from 2 to 35 with a chain of processes:
// each stage reads its first number p (a prime), then forwards
// only the numbers not divisible by p to the next stage.

void
sieve(int in)
{
  int p, n;

  if(read(in, &p, sizeof(p)) != sizeof(p)){
    close(in);
    return;                             // end of the chain
  }
  printf("prime %d\n", p);

  int fds[2];
  pipe(fds);
  if(fork() == 0){
    close(fds[1]);
    close(in);
    sieve(fds[0]);                      // the next stage
    exit(0);
  }
  close(fds[0]);
  while(read(in, &n, sizeof(n)) == sizeof(n)){
    if(n % p != 0)
      write(fds[1], &n, sizeof(n));
  }
  close(in);
  close(fds[1]);                        // EOF for the next stage
  wait(0);
}

int
main(int argc, char *argv[])
{
  int fds[2];
  pipe(fds);
  if(fork() == 0){
    close(fds[1]);
    sieve(fds[0]);
    exit(0);
  }
  close(fds[0]);
  for(int i = 2; i <= 35; i++)
    write(fds[1], &i, sizeof(i));
  close(fds[1]);
  wait(0);
  exit(0);
}
