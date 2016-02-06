/*
   pr47538.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

#ifndef SDCC_mcs51
#include <string.h>

/* PR tree-optimization/47538 */

struct S
{
  double a, b, *c;
  unsigned long d;
};

void
foo (struct S *x, const struct S *y)
{
  const unsigned long n = y->d + 1;
  const double m = 0.25 * (y->b - y->a);
  x->a = y->a;
  x->b = y->b;
  if (n == 1)
    {
      x->c[0] = 0.;
    }
  else if (n == 2)
    {
      x->c[1] = m * y->c[0];
      x->c[0] = 2.0 * x->c[1];
    }
  else
    {
      double o = 0.0, p = 1.0;
      unsigned long i;

      for (i = 1; i <= n - 2; i++)
	{
	  x->c[i] = m * (y->c[i - 1] - y->c[i + 1]) / (double) i;
	  o += p * x->c[i];
	  p = -p;
	}
      x->c[n - 1] = m * y->c[n - 2] / (n - 1.0);
      o += p * x->c[n - 1];
      x->c[0] = 2.0 * o;
    }
}
#endif

void
testTortureExecute (void)
{
#if !(defined __SDCC_mcs51)
  struct S x, y;
  double c[4] = { 10, 20, 30, 40 }, d[4], e[4] = { 118, 118, 118, 118 };

  y.a = 10;
  y.b = 6;
  y.c = c;
  x.c = d;
  y.d = 3;
  memcpy (d, e, sizeof d);
  foo (&x, &y);
  if (d[0] != 0 || d[1] != 20 || d[2] != 10 || d[3] != -10)
    ASSERT (0);
  y.d = 2;
  memcpy (d, e, sizeof d);
  foo (&x, &y);
  if (d[0] != 60 || d[1] != 20 || d[2] != -10 || d[3] != 118)
    ASSERT (0);
  y.d = 1;
  memcpy (d, e, sizeof d);
  foo (&x, &y);
  if (d[0] != -20 || d[1] != -10 || d[2] != 118 || d[3] != 118)
    ASSERT (0);
  y.d = 0;
  memcpy (d, e, sizeof d);
  foo (&x, &y);
  if (d[0] != 0 || d[1] != 118 || d[2] != 118 || d[3] != 118)
    ASSERT (0);
  return;
#endif
}

