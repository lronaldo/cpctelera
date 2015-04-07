/*
   941025-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

long f (long x, long y)
{
  return (x > 1) ? y : (y & 1);
}

void
testTortureExecute (void)
{
  if (f (2L, 0xdecadeL) != 0xdecadeL)
    ASSERT (0);
  return;
}

