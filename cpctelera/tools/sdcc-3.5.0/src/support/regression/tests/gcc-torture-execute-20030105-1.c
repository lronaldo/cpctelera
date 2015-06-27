/*
   20030105-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
foo ()
{
  const int a[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  int i, sum;

  sum = 0;
  for (i = 0; i < sizeof (a) / sizeof (*a); i++)
    sum += a[i];

  return sum;
}

void
testTortureExecute (void)
{
  if (foo () != 28)
    ASSERT (0);
  return;
}
