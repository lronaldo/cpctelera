/*
   20011019-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct { int a; int b[5]; } x;
int *y;

int foo (void)
{
  return y - x.b;
}

void
testTortureExecute (void)
{
  y = x.b;
  if (foo ())
    ASSERT (0);
  return;
}

