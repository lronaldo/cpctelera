/*
pr86231.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* PR tree-optimization/86231 */

#define ONE ((__xdata void *) 1)
#define TWO ((__xdata void *) 2)

int
foo (void *p, int x)
{
  if (p == ONE) return 0;
  if (!p)
    p = x ? TWO : ONE;
  return p == ONE ? 0 : 1;
}

int v[8];

void
testTortureExecute (void)
{
  ASSERT (foo ((void *) 0, 0) == 0);
  ASSERT (foo ((void *) 0, 1) == 1);
  ASSERT (foo (ONE, 0) == 0);
  ASSERT (foo (ONE, 1) == 0);
  ASSERT (foo (TWO, 0) == 1);
  ASSERT (foo (TWO, 1) == 1);
  ASSERT (foo (&v[7], 0) == 1);
  ASSERT (foo (&v[7], 1) == 1);
  return;
}
