/*
   pr61682.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/61682 */

int a, b;
static int *c = &b;

void
testTortureExecute (void)
{
  int *d = &a;
  for (a = 0; a < 12; a++)
    *c |= *d / 9;

  if (b != 1)
    ASSERT (0);
}
