/*
   20000605-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct F { int i; };

void f1(struct F *x, struct F *y)
{
  int timeout = 0;
  for (; ((const struct F*)x)->i < y->i ; x->i++)
    if (++timeout > 5)
      ASSERT (0);
}

void
testTortureExecute (void)
{
  struct F x, y;
  x.i = 0;
  y.i = 1;
  f1 (&x, &y);
  return;
}

