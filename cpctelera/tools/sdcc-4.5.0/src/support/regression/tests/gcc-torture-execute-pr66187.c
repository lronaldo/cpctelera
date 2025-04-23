/*
   pr66187.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR tree-optimization/66187 */

int a = 1, e = -1;
short b, f;

void
testTortureExecute (void)
{
  f = e;
#if 0 // Enable when SDCC supports intermingling
  int g = b < 0 ? 0 : f + b;
  if ((g & -4) < 0)
    a = 0;
  if (a)
    ASSERT (0);
#endif
  return;
}
