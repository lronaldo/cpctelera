/*
   pr68911.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

char a;
int b, c;
short d;

void
testTortureExecute (void)
{
  unsigned e = 2;
  unsigned timeout = 0;

  for (; c < 2; c++)
    {
      int f = ~e / 7;
      if (f)
	a = e = ~(b && d);
      while (e < 94)
	{
	  e++;
	  if (++timeout > 100)
	    goto die;
	}
    }
  return;
die:
  ASSERT(0);
}
