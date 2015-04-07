/*
   931009-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void f ();

void
testTortureExecute (void)
{
  f ();
  return;
}

static void
g (int *out, int size, int lo, int hi)
{
  int j;

  for (j = 0; j < size; j++)
    out[j] = j * (hi - lo);
}


void f ()
{
  int a[2];

  g (a, 2, 0, 1);

  if (a[0] != 0 || a[1] != 1)
    ASSERT (0);
}

