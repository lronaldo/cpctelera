/*
   930513-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

void sub3 (const int *i)
{
}

void eq (int a, int b)
{
  static int i = 0;
  if (a != i)
    ASSERT (0);
  i++;
}

void
testTortureExecute (void)
{
  int i;

  for (i = 0; i < 4; i++)
    {
      const int j = i;
      int k;
      sub3 (&j);
      k = j;
      eq (k, k);
    }
  return;
}

