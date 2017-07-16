/*
   20050713-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: Enable when struct can be returned
/* Test that sibling call is not used if there is an argument overlap.  */

struct S
{
  int a, b, c;
};

int
foo2 (struct S x, struct S y)
{
  if (x.a != 3 || x.b != 4 || x.c != 5)
    ASSERT (0);
  if (y.a != 6 || y.b != 7 || y.c != 8)
    ASSERT (0);
  return 0;
}

int
foo3 (struct S x, struct S y, struct S z)
{
  foo2 (x, y);
  if (z.a != 9 || z.b != 10 || z.c != 11)
    ASSERT (0);
  return 0;
}

int
bar2 (struct S x, struct S y)
{
  return foo2 (y, x);
}

int
bar3 (struct S x, struct S y, struct S z)
{
  return foo3 (y, x, z);
}

int
baz3 (struct S x, struct S y, struct S z)
{
  return foo3 (y, z, x);
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct S a = { 3, 4, 5 }, b = { 6, 7, 8 }, c = { 9, 10, 11 };

  bar2 (b, a);
  bar3 (b, a, c);
  baz3 (c, a, b);
  return;
#endif
}
