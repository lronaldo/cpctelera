/*
   pr79450.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/79450 */
unsigned int
foo (unsigned char x, unsigned long long y)
{
  do
    {
      x &= !y;
      x %= 24;
    }
  while (x < y);
  return x + y;
}

void
testTortureExecute (void)
{
  unsigned int x = foo (1, 0);
  if (x != 1)
    ASSERT (0);
  return;
}
