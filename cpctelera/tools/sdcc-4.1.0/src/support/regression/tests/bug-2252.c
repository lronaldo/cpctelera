/*
   bug-2252.c - the 0xff was seen as an 8-bit quantity, and thus the same as -1, resulting in the addition being replaced by a decrement.
 */

#include <testfwk.h>

int abc(int a)
{
  return a + 0xff;
}

void testBug(void)
{
  ASSERT(abc(0) == 0xff);
}

