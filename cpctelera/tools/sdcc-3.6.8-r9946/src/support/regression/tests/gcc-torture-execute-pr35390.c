/*
   pr35390.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned int foo (int n)
{
  return ~((unsigned int)~n);
}

void
testTortureExecute (void)
{
  if (foo(0) != 0)
    ASSERT (0);
  return;
}
