/*
   pr70586.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/70586 */

int a, e, f;
short b, c, d;

int
foo (int x, int y)
{
  return (y == 0 || (x && y == 1)) ? x : x % y;
}

static short
bar (void)
{
  int i = foo (c, f);
  f = foo (d, 2);
#if 0 // Enable when SDCC intermingles
  int g = foo (b, c);
  int h = foo (g > 0, c);
  c = (3 >= h ^ 7) <= foo (i, c);
  if (foo (e, 1))
    return a;
#endif
  return 0;
}

void
testTortureExecute (void)
{
  bar ();
  return;
}
