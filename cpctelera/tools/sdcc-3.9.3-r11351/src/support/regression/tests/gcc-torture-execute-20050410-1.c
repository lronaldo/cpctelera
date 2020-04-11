/*
   20050410-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int s = 200;
int
foo (void)
{
  return (signed char) (s - 100) - 5;
}
void
testTortureExecute (void)
{
  if (foo () != 95)
    ASSERT (0);
  return;
}
