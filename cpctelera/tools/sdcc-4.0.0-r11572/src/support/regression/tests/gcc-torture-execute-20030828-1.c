/*
   20030828-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

const int *p;

int bar (void)
{
  return *p + 1;
}

void
testTortureExecute (void)
{
  /* Variable 'i' is never used but it's aliased to a global pointer.  The
     alias analyzer was not considering that 'i' may be used in the call to
     bar().  */
  const int i = 5;
  p = &i;
  if (bar() != 6)
    ASSERT (0);
  return;
}

