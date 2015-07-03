/*
   931208-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f ()
{
  unsigned long x, y = 1;

  x = ((y * 8192) - 216) / 16;
  return x;
}

void
testTortureExecute (void)
{
  if (f () != 498)
    ASSERT (0);
  return;
}

