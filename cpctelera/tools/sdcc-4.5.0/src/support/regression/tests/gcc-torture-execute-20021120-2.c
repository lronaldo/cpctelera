/*
   20021120-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int g1, g2;

void foo (int x)
{
  int y;

  if (x)
    y = 793;
  else
    y = 793;
  g1 = 7930 / y;
  g2 = 7930 / x;
}

void
testTortureExecute (void)
{
  foo (793);
  if (g1 != 10 || g2 != 10)
    ASSERT (0);
  return;
}
