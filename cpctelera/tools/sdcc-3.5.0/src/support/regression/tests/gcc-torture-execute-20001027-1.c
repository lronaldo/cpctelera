/*
   20001027-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int x,*p=&x;

void
testTortureExecute (void)
{
#if 0
  int i=0;
  x=1;
  *p=2;
  if (x != 2)
    ASSERT (0);
  return;
#endif
}

