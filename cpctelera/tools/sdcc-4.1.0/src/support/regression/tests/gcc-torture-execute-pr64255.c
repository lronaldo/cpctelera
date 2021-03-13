/*
   pr64255.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/64255 */

void
bar (long i, unsigned long j)
{
  if (i != 1 || j != 1)
    ASSERT (0);
}

void
foo (long i)
{
  unsigned long j;

  if (!i)
    return;
  j = i >= 0 ? (unsigned long) i : - (unsigned long) i;
  if ((i >= 0 ? (unsigned long) i : - (unsigned long) i) != j)
    ASSERT (0);
  bar (i, j);
}

void
testTortureExecute (void)
{
  foo (1);
  return;
}
