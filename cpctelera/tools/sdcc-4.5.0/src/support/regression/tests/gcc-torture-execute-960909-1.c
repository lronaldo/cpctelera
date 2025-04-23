/*
   960909-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
ffs (int x)
{
  int bit, mask;

  if (x == 0)
    return 0;

  for (bit = 1, mask = 1; !(x & mask); bit++, mask <<= 1)
    ;

  return bit;
}

void f (int x)
{
  int y;
  y = ffs (x) - 1;
  if (y < 0) 
    ASSERT (0);
}

void
testTortureExecute (void)
{
  f (1);
  return;
}

