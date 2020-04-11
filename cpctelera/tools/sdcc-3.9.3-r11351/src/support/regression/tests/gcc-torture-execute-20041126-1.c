/*
   20041126-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdlib.h>

void
check (int *p)
{
  int i;
  for (i = 0; i < 5; ++i)
    if (p[i])
      ASSERT (0);
  for (; i < 10; ++i)
    if (p[i] != i + 1)
      ASSERT (0);
}

void
testTortureExecute (void)
{
  int a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  int i;

  for (i = -5; i < 0; i++)
    a[abs (i - 10) - 11] = 0;
  check (a);
  return;
}
