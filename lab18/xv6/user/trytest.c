// trytest.c: shows both answers of sem_trydown (optional part of Lab 13).
#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  sem_init(4, 1);
  printf("value 1: sem_trydown(4) = %d   (took the unit)\n", sem_trydown(4));
  printf("value 0: sem_trydown(4) = %d  (nothing to take, did not sleep)\n", sem_trydown(4));
  sem_up(4);
  printf("after sem_up(4): sem_trydown(4) = %d\n", sem_trydown(4));
  exit(0);
}
