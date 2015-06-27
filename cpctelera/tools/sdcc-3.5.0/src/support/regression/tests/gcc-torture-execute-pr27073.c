/*
   pr27073.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

void
foo (int *p, int d1, int d2, int d3,
     short count, int s1, int s2, int s3, int s4, int s5)
{
  int n = count;
  while (n--)
    {
      *p++ = s1;
      *p++ = s2;
      *p++ = s3;
      *p++ = s4;
      *p++ = s5;
    }
}

void
testTortureExecute (void)
{
  int x[10], i;

  foo (x, 0, 0, 0, 2, 100, 200, 300, 400, 500);
  for (i = 0; i < 10; i++)
    if (x[i] != (i % 5 + 1) * 100)
      ASSERT (0);
  return;
}

