/*
   960219-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
f (int i)
{
  ASSERT(((1 << i) & 1) != 0);
}

void
testTortureExecute (void)
{
  f (0);
  return;
}

