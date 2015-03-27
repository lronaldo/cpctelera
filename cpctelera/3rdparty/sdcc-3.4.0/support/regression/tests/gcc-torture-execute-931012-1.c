/*
   931012-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f (int b, int c)
{
  if (b != 0 && b != 1 && c != 0)
    b = 0;
  return b;
}

void
testTortureExecute (void)
{
  if (!f (1, 2))
    ASSERT (0);
  return;
}

