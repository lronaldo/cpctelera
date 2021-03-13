/*
   20040706-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
  int i;
  for (i = 0; i < 10; i++)
    continue;
  if (i < 10)
    ASSERT (0);
  return;
}
