/*
   pr68250.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 84
#endif

/* PR rtl-optimization/68250 */

signed char a, b, h, k, l, m, o;
short c, d, n;
int e, f, g, j, q;

void
fn1 (void)
{
  int p = b || a;
  n = o > 0 || d > 1 >> o ? d : d << o;
  for (; j; j++)
    m = c < 0 || m || c << p;
  l = f + 1;
  for (; f < 1; f = 1)
    k = h + 1;
}

void
fn2 (int k)
{
  ASSERT (k == 1);
}

void
testTortureExecute (void)
{
  signed char i;
  for (; e < 1; e++)
    {
      fn1 ();
      if (k)
        i = k;
      if (i > q)
        g = 0;
    }
  fn2 (k);
  return;
}
