/*
   pr71631.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/71631 */

volatile char v;
int a = 1, b = 1, c = 1;

void
foo (const char *s)
{
  while (*s++)
    v = *s;
}

void
testTortureExecute (void)
{
  volatile int d = 1;
  volatile int e = 1;
  int f = 1 / a;
  int g = 1U < f;
  int h = 2 + g;
  int i = 3 % h;
  int j = e && b;
  int k = 1 == c;
  int l = d != 0;
  short m = (short) (-1 * i * l);
  short x = j * (k * m);
  if (i == 1)
    foo ("AB");
  if (x != -1)
    ASSERT (0);
  return;
}
