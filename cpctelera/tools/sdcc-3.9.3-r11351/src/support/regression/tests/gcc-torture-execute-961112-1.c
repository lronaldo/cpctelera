/*
   961112-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f (int x)
{
  if (x != 0 || x == 0)
    return 0;
  return 1;
}

void
testTortureExecute (void)
{
  if (f (3))
    ASSERT (0);
  return;
}

