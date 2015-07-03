/*
   930529-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int dd (int x, int d) { return x / d; }

void
testTortureExecute (void)
{
  int i;
  for (i = -3; i <= 3; i++)
    {
      if (dd (i, 1) != i / 1)
	ASSERT (0);
      if (dd (i, 2) != i / 2)
	ASSERT (0);
      if (dd (i, 3) != i / 3)
	ASSERT (0);
      if (dd (i, 4) != i / 4)
	ASSERT (0);
      if (dd (i, 5) != i / 5)
	ASSERT (0);
      if (dd (i, 6) != i / 6)
	ASSERT (0);
      if (dd (i, 7) != i / 7)
	ASSERT (0);
      if (dd (i, 8) != i / 8)
	ASSERT (0);
    }
  for (i = ((unsigned) ~0 >> 1) - 3; i <= ((unsigned) ~0 >> 1) + 3; i++)
    {
      if (dd (i, 1) != i / 1)
	ASSERT (0);
      if (dd (i, 2) != i / 2)
	ASSERT (0);
      if (dd (i, 3) != i / 3)
	ASSERT (0);
      if (dd (i, 4) != i / 4)
	ASSERT (0);
      if (dd (i, 5) != i / 5)
	ASSERT (0);
      if (dd (i, 6) != i / 6)
	ASSERT (0);
      if (dd (i, 7) != i / 7)
	ASSERT (0);
      if (dd (i, 8) != i / 8)
	ASSERT (0);
    }
  return;
}

