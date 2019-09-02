/*
pr85529-2.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* PR tree-optimization/85529 */

int
foo (int x)
{
  x &= 63;
  x -= 50;
  x |= 1;
  if (x < 0)
    return 1;
  int y = x >> 2;
  if (x >= y)
    return 1;
  return 0;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && (__GNUC__ < 8))
  int i;
  for (i = 0; i < 63; i++)
    if (foo (i) != 1)
      ASSERT (0);
  return;
#endif
}
