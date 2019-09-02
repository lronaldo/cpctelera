/*
   pr15262.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct X { int i; };

int
foo (struct X *p, int *q, int a, int b)
{
  struct X x, y;
  if (a)
    p = &x;
  if (b)
    q = &x.i;
  else
    q = &y.i;
  *q = 1;
  return p->i; 
}

void
testTortureExecute (void)
{
  if (foo((void *)0, (void *)0, 1, 1) != 1)
    ASSERT (0);
  return;
}

