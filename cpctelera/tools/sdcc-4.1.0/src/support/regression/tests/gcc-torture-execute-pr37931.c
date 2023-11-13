/*
   pr37931.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR middle-end/37931 */


int
foo (int a, unsigned int b)
{
  return (a | 1) & (b | 1);
}

void
testTortureExecute (void)
{
  if (foo (6, 0xc6) != 7)
    ASSERT (0);
  if (foo (0x80, 0xc1) != 0x81)
    ASSERT (0);
  if (foo (4, 4) != 5)
    ASSERT (0);
  if (foo (5, 4) != 5)
    ASSERT (0);
  return;
}
