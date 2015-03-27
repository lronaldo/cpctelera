/*
   20060412-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#ifndef __SDCC_mcs51
struct S
{
  long o;
};

struct T
{
  long o;
  struct S m[82];
};

struct T t;
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_mcs51
  struct S *p, *q;

  p = (struct S *) &t;
  p = &((struct T *) p)->m[0];
  q = p + 82;
  while (--q > p)
    q->o = -1;
  q->o = 0;

  if (q > p)
    ASSERT (0);
  if (q - p > 0)
    ASSERT (0);
  return;
#endif
}
