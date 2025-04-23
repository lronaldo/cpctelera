/*
   960321-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

char a[10] = "deadbeef";

char
acc_a (long i)
{
  return a[i-2000000000L];
}

void
testTortureExecute (void)
{
  if (acc_a (2000000000L) != 'd')
    ASSERT (0);
  return;
}

