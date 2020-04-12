/*
   mod-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c89
#endif

int f (int x, int y)
{
  if (x % y != 0)
    ASSERT(0);
}

void
testTortureExecute (void)
{
  f (-5, 5);
  return;
}

