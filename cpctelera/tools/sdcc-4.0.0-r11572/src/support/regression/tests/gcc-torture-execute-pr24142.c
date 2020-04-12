/*
   pr24142.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f (int a, int b)
{
  if (a == 1)
    a = 0;
  if (b == 0)
    a = 1;
  if (a != 0)
    return 0;
  return 1;
}

void
testTortureExecute (void)
{
  if (f (1, 1) != 1)
    ASSERT (0);
  return;
}

