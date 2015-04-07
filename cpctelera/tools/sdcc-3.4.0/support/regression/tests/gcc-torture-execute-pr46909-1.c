/*
   pr46909.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/46909 */

int
foo (unsigned int x)
{
  if (! (x == 4 || x == 6) || (x == 2 || x == 6))
    return 1;
  return -1;
}

void
testTortureExecute (void)
{
  int i;
  for (i = -10; i < 10; i++)
    if (foo (i) != 1 - 2 * (i == 4))
      ASSERT (0);
  return;
}

