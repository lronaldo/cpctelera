/*
   950612-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
unsigned int
f1 (int diff)
{
  return ((unsigned int) (diff < 0 ? -diff : diff));
}

unsigned int
f2 (unsigned int diff)
{
  return ((unsigned int) ((signed int) diff < 0 ? -diff : diff));
}

unsigned long long
f3 (long long diff)
{
  return ((unsigned long long) (diff < 0 ? -diff : diff));
}

unsigned long long
f4 (unsigned long long diff)
{
  return ((unsigned long long) ((signed long long) diff < 0 ? -diff : diff));
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  int i;
  for (i = 0; i <= 10; i++)
    {
      if (f1 (i) != i)
	ASSERT (0);
      if (f1 (-i) != i)
	ASSERT (0);
      if (f2 (i) != i)
	ASSERT (0);
      if (f2 (-i) != i)
	ASSERT (0);
      if (f3 ((long long) i) != i)
	ASSERT (0);
      if (f3 ((long long) -i) != i)
	ASSERT (0);
      if (f4 ((long long) i) != i)
	ASSERT (0);
      if (f4 ((long long) -i) != i)
	ASSERT (0);
    }
  return;
#endif
}

