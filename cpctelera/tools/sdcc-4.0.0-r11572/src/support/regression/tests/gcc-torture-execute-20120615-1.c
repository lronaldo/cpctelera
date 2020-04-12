/*
   20120615-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void 
     test1(int i)
{
  if (i == 12)
    return;
  if (i != 17)
    {
      if (i == 15)
	return;
      ASSERT (0);
    }
}

void
testTortureExecute (void)
{ test1 (15);}

