/*
   20050613-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/22043 */

struct A { int i; int j; int k; int l; };
struct B { struct A a; int r[1]; };
struct C { struct A a; int r[0]; };
struct D { struct A a; int r[]; };

void
foo (struct A *x)
{
  ASSERT (x->i == 0);
  ASSERT (x->j == 5);
  ASSERT (x->k == 0);
  ASSERT (x->l == 0);
}

void
testTortureExecute (void)
{
  struct B b = { .a.j = 5 };
  struct C c = { .a.j = 5 };
  struct D d = { .a.j = 5 };
  foo (&b.a);
  foo (&c.a);
  foo (&d.a);
  return;
}
