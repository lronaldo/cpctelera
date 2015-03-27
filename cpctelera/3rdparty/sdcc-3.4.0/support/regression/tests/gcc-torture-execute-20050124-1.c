/*
   20050124-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/19579 */

int
foo (int i, int j)
{
  int k = i + 1;

  if (j)
    {
      if (k > 0)
	k++;
      else if (k < 0)
	k--;
    }

  return k;
}

void
testTortureExecute (void)
{
  if (foo (-2, 0) != -1)
    ASSERT (0);
  if (foo (-1, 0) != 0)
    ASSERT (0);
  if (foo (0, 0) != 1)
    ASSERT (0);
  if (foo (1, 0) != 2)
    ASSERT (0);
  if (foo (-2, 1) != -2)
    ASSERT (0);
  if (foo (-1, 1) != 0)
    ASSERT (0);
  if (foo (0, 1) != 2)
    ASSERT (0);
  if (foo (1, 1) != 3)
    ASSERT (0);
  return;
}
