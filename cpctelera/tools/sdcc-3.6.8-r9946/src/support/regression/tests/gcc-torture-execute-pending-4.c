/*
   pending-4.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c89
#endif

#pragma disable_warning 85

void dummy (int *x, int y)
{}

void
testTortureExecute (void)
{
  int number_columns=9;
  int cnt0 = 0;
  int cnt1 = 0;
  int i,A1;

  for (i = number_columns-1; i != 0; i--)
    {         
      if (i == 1)
	{
	  dummy(&A1, i);
	  cnt0++;
	}
      else
	{
          dummy(&A1, i-1);
          cnt1++;
	}
    }
  if (cnt0 != 1 || cnt1 != 7)
    ASSERT(0);
  return;
}
