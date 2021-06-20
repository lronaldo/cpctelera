/*
   991202-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
f1 ()
{
  unsigned long x, y = 1;

  x = ((y * 8192) - 216) % 16;
  return x;
}

void
testTortureExecute (void)
{
  if (f1 () != 8)
    ASSERT (0);
  return;
}

