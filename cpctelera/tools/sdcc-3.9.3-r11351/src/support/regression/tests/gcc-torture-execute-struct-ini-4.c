/*
   struct-ini-4.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct s {
  int a[3];
  int c[3];
};

struct s s = {
  .c = {1, 2, 3}
};

void
testTortureExecute (void)
{
  if (s.c[0] != 1)
    ASSERT(0);
  return;
}

