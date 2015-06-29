/*
   20000706-5.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports.
#if 0
struct baz { int a, b, c; };

struct baz *c;

void bar(int b)
{
  if (c->a != 1 || c->b != 2 || c->c != 3 || b != 4)
    ASSERT(0);
}

void foo(struct baz a, int b)
{
  c = &a;
  bar(b);
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct baz a;
  a.a = 1;
  a.b = 2;
  a.c = 3;
  foo(a, 4);
  return;
#endif
}

