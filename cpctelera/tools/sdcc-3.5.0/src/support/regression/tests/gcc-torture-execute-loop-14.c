/*
   loop-14.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int a3[3];

void f(int *a)
{
  int i;

  for (i=3; --i;)
    a[i] = 42 / i;
}

void
testTortureExecute (void)
{
  f(a3);

  if (a3[1] != 42 || a3[2] != 21)
    ASSERT (0);

  return;
}
