/*
   930429-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
f (int b)
{
  return (b >> 1) > 0;
}

void
testTortureExecute (void)
{
  if (!f (9))
    ASSERT (0);
  if (f (-9))
    ASSERT (0);
  return;
}

