/*
   990117-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

int
foo (int x, int y, int i, int j)
{
  double tmp1 = ((double) x / y);
  double tmp2 = ((double) i / j);

  return tmp1 < tmp2;
}

void
testTortureExecute (void)
{
  if (foo (2, 24, 3, 4) == 0)
    ASSERT (0);
  return;
}

