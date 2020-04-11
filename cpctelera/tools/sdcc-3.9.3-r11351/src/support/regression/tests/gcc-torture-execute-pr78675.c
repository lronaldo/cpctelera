/*
   pr78675.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 84
#endif

/* PR tree-optimization/78675 */

long int a;

long int
foo (long int x)
{
  long int b;
  while (a < 1)
    {
      b = a && x;
      ++a;
    }
  return b;
}

void
testTortureExecute (void)
{
  ASSERT (foo (0) == 0);
  a = 0;
  ASSERT (foo (1) == 0);
  a = 0;
  ASSERT (foo (25) == 0);
  a = -64;
  ASSERT (foo (0) == 0);
  a = -64;
  ASSERT (foo (1) == 0);
  a = -64;
  ASSERT (foo (25) == 0);
  return;
}
