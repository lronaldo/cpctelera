/*
   loop-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int a[2];

void f (int b)
{
  unsigned int i;
  for (i = 0; i < b; i++)
    a[i] = i - 2;
}

void
testTortureExecute (void)
{
  a[0] = a[1] = 0;
  f (2);
  if (a[0] != -2 || a[1] != -1)
    ASSERT (0);
  return;
}
