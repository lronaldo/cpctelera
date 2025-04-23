/*
   20071108-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

/* PR tree-optimization/32575 */

struct S
{
  void *s1, *s2;
  unsigned char s3, s4, s5;
};

void *
foo (void)
{
  static struct S s;
  return &s;
}

void *
bar ()
{
  return (void *) 0;
}

struct S *
test (void *a, void *b)
{
  struct S *p, q;
  p = foo ();
  if (p == 0)
    {
      p = &q;
      memset (p, 0, sizeof (*p));
    }
  p->s1 = a;
  p->s2 = b;
  if (p == &q)
    p = 0;
  return p;
}

void
testTortureExecute (void)
{
  int a;
  int b;
  struct S *z = test ((void *) &a, (void *) &b);
  if (z == 0 || z->s1 != (void *) &a || z->s2 != (void *) &b || z->s3 || z->s4)
    ASSERT (0);
  return;
}
