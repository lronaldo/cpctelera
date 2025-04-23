/*
   20021118-3.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
foo (int x)
{
  if (x == -2 || -x - 100 >= 0)
    ASSERT (0);
  return 0;
}
           
void
testTortureExecute (void)
{
  foo (-3);
  foo (-99);
  return;
}
