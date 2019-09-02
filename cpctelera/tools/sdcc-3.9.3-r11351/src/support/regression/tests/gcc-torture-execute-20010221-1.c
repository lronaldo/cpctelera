/*
   20010221-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int n = 2;

void
testTortureExecute (void)
{
  int i, x = 45;

  for (i = 0; i < n; i++)
    {
      if (i != 0)
	x = ( i > 0 ) ? i : 0;
    }

  if (x != 1)
    ASSERT (0);
  return;
}

