/*
   921104-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
  unsigned long val = 1;

  if (val > (unsigned long) ~0)
    ASSERT(0);
  return;
}
