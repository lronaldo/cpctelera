/*
   bug-2228.c was broken by a missing notUsedFrom() test in z80 and r2k peepholes.
 */

#include <testfwk.h>

char i, j = 4;

void testBug(void)
{
  i = 0;

  while (j--)
    i = (i == 3) ? 0 : (i + 1);

  ASSERT(!i);
}

