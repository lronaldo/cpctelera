/*
   20030606-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

int * foo (int *x, int b)
{

  *(x++) = 55;
  if (b)
    *(x++) = b;

  return x;
}

void
testTortureExecute (void)
{
  int a[5];

  memset (a, 1, sizeof (a));

  if (foo (a, 0) - a != 1 || a[0] != 55 || a[1] != a[4])
    ASSERT (0);

  memset (a, 1, sizeof (a));

  if (foo (a, 2) - a != 2 || a[0] != 55 || a[1] != 2)
    ASSERT (0);

  return;
}

