/*
   20000503-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned long
sub (int a)
{
  return ((0 > a - 2) ? 0 : a - 2) * sizeof (long);
}

void
testTortureExecute (void)
{
  if (sub (0) != 0)
    ASSERT (0);

  return;
}

