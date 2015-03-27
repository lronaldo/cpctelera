/*
   20090623-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int *restrict x;

int foo (int y)
{
  *x = y;
  return *x;
}

void
testTortureExecute (void)
{
  int i = 0;
  x = &i;
  if (foo(1) != 1)
    ASSERT (0);
  return;
}

