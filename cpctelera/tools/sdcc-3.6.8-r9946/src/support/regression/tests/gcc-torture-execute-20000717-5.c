/*
   20000717-5.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
typedef struct trio { int a, b, c; } trio;

int
bar (int i, int j, int k, trio t)
{
  if (t.a != 1 || t.b != 2 || t.c != 3 ||
      i != 4 || j != 5 || k != 6)
    ASSERT (0);
}

int
foo (trio t, int i, int j, int k)
{
  return bar (i, j, k, t);
}
#endif

void
testTortureExecute (void)
{
#if 0
  trio t = { 1, 2, 3 };

  foo (t, 4, 5, 6);
  return;
#endif
}

