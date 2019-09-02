/*
   pr19005.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/19005 */

int v, s;

void
bar (int a, int b)
{
  unsigned char x = v;

  if (!s)
    {
      if (a != x || b != (unsigned char) (x + 1))
        ASSERT (0);
    }
  else if (a != (unsigned char) (x + 1) || b != x)
    ASSERT (0);
  s ^= 1;
}

int
foo (int x)
{
  unsigned char a = x, b = x + 1;

  bar (a, b);
  a ^= b; b ^= a; a ^= b;
  bar (a, b);
  return 0;
}

void
testTortureExecute (void)
{
  for (v = -10; v < 266; v++)
    foo (v);
  return;
}
