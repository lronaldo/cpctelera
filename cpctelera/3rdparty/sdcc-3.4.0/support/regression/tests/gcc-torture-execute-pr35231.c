/*
   pr35231.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
foo(int bits_per_pixel, int depth)
{
  if ((bits_per_pixel | depth) == 1)
    ASSERT (0);
  return bits_per_pixel;
}

void
testTortureExecute (void)
{
  if (foo(2, 0) != 2)
    ASSERT (0);
  return;
}
