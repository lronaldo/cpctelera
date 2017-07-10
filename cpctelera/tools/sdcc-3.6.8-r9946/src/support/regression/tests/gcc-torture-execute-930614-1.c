/*
   930614-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#pragma disable_warning 85
#endif

void f (double *ty)
{
  *ty = -1.0;
}

void
testTortureExecute (void)
{
  double foo[6];
  double tx = 0.0, ty, d;

  f (&ty);

  if (ty < 0)
    ty = -ty;
  d = (tx > ty) ? tx : ty;
  if (ty != d)
    ASSERT (0);
  return;
}

