/*
   pr40747.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR middle-end/40747 */

int
foo (int i)
{
  return (i < 4 && i >= 0) ? i : 4;
}

void
testTortureExecute (void)
{
  if (foo (-1) != 4) ASSERT (0);
  if (foo (0) != 0) ASSERT (0);
  if (foo (1) != 1) ASSERT (0);
  if (foo (2) != 2) ASSERT (0);
  if (foo (3) != 3) ASSERT (0);
  if (foo (4) != 4) ASSERT (0);
  if (foo (5) != 4) ASSERT (0);
  return;
}

