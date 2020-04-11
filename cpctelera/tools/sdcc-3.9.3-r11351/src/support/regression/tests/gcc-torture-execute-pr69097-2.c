/*
   pr69097-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

/* PR tree-optimization/69097 */

int
f1 (int x, int y)
{
  return x % y;
}

int
f2 (int x, int y)
{
  return x % -y;
}

int
f3 (int x, int y)
{
  int z = -y;
  return x % z;
}

void
testTortureExecute (void)
{
  if (f1 (-INT_MAX - 1, 1) != 0
      || f2 (-INT_MAX - 1, -1) != 0
      || f3 (-INT_MAX - 1, -1) != 0)
    ASSERT(0);
  return;
}
