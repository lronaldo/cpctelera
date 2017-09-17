/*
   20040423-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: enable when variable-length arrays are asupported!
#include <string.h>

int
sub1 (int i, int j)
{
  typedef struct
  {
   int  c[i+2];
  }c;
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
  typedef struct
  {
   int  c[22];
  }c;
  if (sub1 (20, 3) != sizeof (c)*3)
    ASSERT (0);

  return;
#endif
}
