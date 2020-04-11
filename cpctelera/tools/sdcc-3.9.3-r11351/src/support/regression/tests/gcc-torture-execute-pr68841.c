/*
   pr68841.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Bug #2874
static inline int
foo (int *x, int y)
{
  int z = *x;
  while (y > z)
    z *= 2;
  return z;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Bug #2874
  int i;
  for (i = 1; i < 17; i++)
    {
      int j;
      int k;
      j = foo (&i, 7);
      if (i >= 7)
	k = i;
      else if (i >= 4)
	k = 8 + (i - 4) * 2;
      else if (i == 3)
	k = 12;
      else
	k = 8;
      if (j != k)
	ASSERT (0);
    }
  return;
#endif
}
