/*
   pr15262-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* TODO: Enable this once we can pass structures as function arguments! */
#if 0
/* PR 15262.  Similar to pr15262-1.c but with no obvious addresses
   being taken in function foo().  Without IPA, by only looking inside
   foo() we cannot tell for certain whether 'q' and 'b' alias each
   other.  */
struct A
{
  int t;
  int i;
};

struct B
{
  int *p;
  float b;
};

float X;

foo (struct B b, struct A *q, float *h)
{
  X += *h;
  *(b.p) = 3;
  q->t = 2;
  return *(b.p);
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct A a;
  struct B b;

  b.p = &a.t;
  if (foo (b, &a, &X) == 3)
    ASSERT (0);

  return;
#endif
}
