/*
   20040411-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: Enable when variable-length arrays are supported!
#include <string.h>

int
sub1 (int i, int j)
{
  typedef int c[i+2];
  int x[10], y[10];

  if (j == 2)
    {
      memcpy (x, y, 10 * sizeof (int));
      return sizeof (c);
    }
  else
    return sizeof (c) * 3;
}
#endif

void
testTortureExecute (void)
{
#if 0
  if (sub1 (20, 3) != 66 * sizeof (int))
    ASSERT (0);

  return;
#endif
}
