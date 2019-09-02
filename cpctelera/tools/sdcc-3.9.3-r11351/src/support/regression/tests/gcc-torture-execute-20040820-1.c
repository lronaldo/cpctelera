/*
   20040820-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/17099 */

void
check (int a)
{
  if (a != 1)
    ASSERT (0);
}

void
test (int a, int b)
{
  check ((a ? 1 : 0) | (b ? 2 : 0));
}

void
testTortureExecute (void)
{
  test (1, 0);
  return;
}
