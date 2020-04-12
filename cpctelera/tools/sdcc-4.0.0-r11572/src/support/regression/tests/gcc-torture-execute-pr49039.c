/*
   pr49039.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/49039 */
int cnt;

void
foo (unsigned int x, unsigned int y)
{
  unsigned int minv, maxv;
  if (x == 1 || y == -2U)
    return;
  minv = x < y ? x : y;
  maxv = x > y ? x : y;
  if (minv == 1)
    ++cnt;
  if (maxv == -2U)
    ++cnt;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && defined (__GNUC_MINOR__) && (__GNUC__ < 5))
  foo (-2U, 1);
  if (cnt != 2)
    ASSERT (0);
  return;
#endif
}

