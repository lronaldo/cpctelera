/*
   20010422-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned int foo(unsigned int x)
{
  if (x < 5)
    x = 4;
  else
    x = 8;
  return x;
}

void
testTortureExecute (void)
{
  if (foo (8) != 8)
    ASSERT (0);
  return;
}

