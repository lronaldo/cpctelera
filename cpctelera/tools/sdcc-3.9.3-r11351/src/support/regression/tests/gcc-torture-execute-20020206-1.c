/*
   20020206-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: Enable when struct can be returned!
struct A {
  unsigned int a, b, c;
};

struct A bar (void)
{
  return (struct A) { 176, 52, 31 };
}

void baz (struct A *a)
{
  if (a->a != 176 || a->b != 52 || a->c != 31)
    ASSERT (0);
}
#endif
void
testTortureExecute (void)
{
#if 0
  struct A d;

  d = ({ ({ bar (); }); });
  baz (&d);
#endif
}
