/*
   loop-8.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 93 // Using float for double.

double a[3] = { 0.0, 1.0, 2.0 };

void bar (int x, double *y)
{
  if (x || *y != 1.0)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  double c;
  int d;
  for (d = 0; d < 3; d++)
  {
    c = a[d];
    if (c > 0.0) goto e;
  }
  bar(1, &c);
  return;
e:
  bar(0, &c);
  return;
}
