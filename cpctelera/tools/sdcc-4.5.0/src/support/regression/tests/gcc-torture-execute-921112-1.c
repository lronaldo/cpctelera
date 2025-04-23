/*
   921112-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

// TODO: Enable when sdcc supports struct!
#if 0
union u {
  struct { int i1, i2; } t;
  double d;
} x[2], v;

void f (union u *x, union u *v)
{
  *++x = v;
}
#endif

void
testTortureExecute (void)
{
#if 0
  x[1].t.i1 = x[1].t.i2 = 0;
  v.t.i1 = 1;
  v.t.i2 = 2;
  f (x, v);
  if (x[1].t.i1 != 1 || x[1].t.i2 != 2)
    ASSERT (0);
  return;
#endif
}

