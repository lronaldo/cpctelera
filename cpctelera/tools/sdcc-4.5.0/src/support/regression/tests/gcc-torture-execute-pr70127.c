/*
   pr70127.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/70127 */

struct S { int f; signed int g : 2; } a[1], c = {5, 1}, d;
short b;

void
foo (int x)
{
  if (x != 1)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  while (b++ <= 0)
    {
      struct S e = {1, 1};
      d = e = a[0] = c;
    }
  foo (a[0].g);
  return;
}
