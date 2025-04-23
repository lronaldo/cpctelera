/*
   pr15262.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int j;

void bla (int *r)
{
  int *p, *q;

  p = q = r;
  if (!p)
    p = &j;
  
  if (p != q)
    j = 1;
}

void
testTortureExecute (void)
{
  bla (0);
  if (!j)
    ASSERT (0);
  return;
}

