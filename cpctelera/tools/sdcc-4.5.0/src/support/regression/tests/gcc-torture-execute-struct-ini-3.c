/*
   struct-ini-3.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct
{
  unsigned int f1:1, f2:1, f3:3, f4:3, f5:2, f6:1, f7:1;
} result = {1, 1, 7, 7, 3, 1, 1};

void
testTortureExecute (void)
{
  if ((result.f3 & ~7) != 0 || (result.f4 & ~7) != 0)
    ASSERT (0);
  return;
}

