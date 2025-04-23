/*
   pr642602.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/64260 */

int a = 1, b;

void
foo (char p)
{
  int t = 0;
  for (; b < 1; b++)
    {
      int *s = &a;
      if (--t)
	*s &= p;
      *s &= 1;
    }
}

void
testTortureExecute (void)
{
  foo (0);
  if (a != 0)
    ASSERT (0);
  return;
}
