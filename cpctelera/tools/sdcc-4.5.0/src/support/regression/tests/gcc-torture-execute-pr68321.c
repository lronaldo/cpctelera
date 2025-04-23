/*
   pr68321.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 84
#endif

/* PR rtl-optimization/68321 */

int e = 1, u = 5, t2, t5, i, k;
int a[1], b, m;
char n, t;

int
fn1 (int p1)
{
  int g[1];
  for (;;)
    {
      if (p1 / 3)
        for (; t5;)
          u || n;
      t2 = p1 & 4;
      if (b + 1)
        return 0;
      u = g[0];
    }
}

void
testTortureExecute (void)
{
  for (; e >= 0; e--)
    {
      char c;
      if (!m)
        c = t;
      fn1 (c);
    }

  ASSERT (a[t2] == 0);

  return;
}
