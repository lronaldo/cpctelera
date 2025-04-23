/*
   pr65216.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR tree-optimization/65216 */

int a, b = 62, e;
volatile int c, d;

void
testTortureExecute (void)
{
  int f = 0;
  for (a = 0; a < 2; a++)
    {
      b &= (8 ^ f) & 1;
      for (e = 0; e < 6; e++)
	if (c)
	  f = d;
    }
  if (b != 0)
    ASSERT (0);
  return;
}
