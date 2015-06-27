/*
   20000717-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Todo: Enable when sdcc supports struct
#if 0
typedef struct trio { int a, b, c; } trio;

int
bar (int i, trio t)
{
  if (t.a == t.b || t.a == t.c)
    ASSERT (0);
}

int
foo (trio t, int i)
{
  return bar (i, t);
}
#endif

void
testTortureExecute (void)
{
#if 0
  trio t = { 1, 2, 3 };

  foo (t, 4);
  return;
#endif
}

