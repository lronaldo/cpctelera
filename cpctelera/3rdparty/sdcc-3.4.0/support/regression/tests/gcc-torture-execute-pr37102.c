/*
   pr37102.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned int a, b = 1, c;

void
foo (int x)
{
  if (x != 5)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  unsigned int d, e;
  for (d = 1; d < 5; d++)
    if (c)
      a = b;
  a = b;
  e = a << 1;
  if (e)
    e = (e << 1) ^ 1;
  foo (e);
  return;
}

