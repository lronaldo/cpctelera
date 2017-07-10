/*
   980602-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
  int i;
  for (i = 1; i < 100; i++)
    ;
  if (i == 100) 
    return;
  ASSERT (0);
}

