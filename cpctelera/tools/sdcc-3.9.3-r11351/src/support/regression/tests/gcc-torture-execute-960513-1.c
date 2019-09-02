/*
   960513-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long double!
#if 0
long double
f (d, i)
     long double d;
     int i;
{
  long double e;

  d = -d;
  e = d;
  if (i == 1)
    d *= 2;
  d -= e * d;
  d -= e * d;
  d -= e * d;
  d -= e * d;
  d -= e * d;
  return d;
}
#endif

void
testTortureExecute (void)
{
#if 0
  if (! (int) (f (2.0L, 1)))
    ASSERT (0);
  return;
#endif
}

