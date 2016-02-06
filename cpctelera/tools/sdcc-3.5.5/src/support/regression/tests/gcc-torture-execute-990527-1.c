/*
   990527-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int sum;

void
g (int i)
{
  sum += i;
}

void
f(int j)
{
  int i;

  for (i = 0; i < 9; i++)
    {
      j++;
      g (j);
      j = 9;
    }
}

void
testTortureExecute (void)
{
  f (0);
  if (sum != 81)
    ASSERT (0);
  return;
}

