/*
   pr48717.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/48717 */

int v = 1, w;

unsigned short
foo (unsigned short x, unsigned short y)
{
  return x + y;
}

void
bar (void)
{
  v = foo (~w, w);
}

void
testTortureExecute (void)
{
  bar ();
  if (v != (unsigned short) -1)
    ASSERT (0);
  return;
}

