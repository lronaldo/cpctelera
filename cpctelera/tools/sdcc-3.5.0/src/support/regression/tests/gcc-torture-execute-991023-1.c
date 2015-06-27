/*
   991023-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int blah;
foo()
{
  int i;

  for (i=0 ; i< 7 ; i++)
    {
      if (i == 7 - 1)
	blah = 0xfcc;
      else
	blah = 0xfee;
    }
  return blah;
}


void
testTortureExecute (void)
{
  if (foo () != 0xfcc)
    ASSERT (0);
  return;
}

