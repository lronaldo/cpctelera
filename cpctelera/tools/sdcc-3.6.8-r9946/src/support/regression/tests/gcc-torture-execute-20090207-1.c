/*
   20090207-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int foo(int i)
{
  int a[32];
  a[1] = 3;
  a[0] = 1;
  a[i] = 2;
  return a[0];
}

void
testTortureExecute (void)
{
  if (foo (0) != 2
      || foo (1) != 1)
    ASSERT (0);
  return;
}

