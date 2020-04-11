/*
   20020920-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: enable when support struct is complete!
struct B
{
  int x;
  int y;
};

struct A
{
  int z;
  struct B b;
};

struct A
f ()
{
  struct B b = { 0, 1 };
  struct A a = { 2, b };
  return a;
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct A a = f ();
  if (a.z != 2 || a.b.x != 0 || a.b.y != 1)
    ASSERT (0);
  return;
#endif
}
