/*
   loop-7.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void foo (unsigned int n)
{
  int i, j = -1;

  for (i = 0; i < 10 && j < 0; i++)
    {
      if ((1UL << i) == n)
	j = i;
    }

  if (j < 0)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  foo (64);
  return;
}
