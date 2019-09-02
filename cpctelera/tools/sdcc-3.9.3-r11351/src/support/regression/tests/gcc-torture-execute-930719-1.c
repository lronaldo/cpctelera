/*
   930719-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
f (int foo, int bar, int com)
{
  unsigned align = 0;
  if (foo)
    return 0;
  while (1)
    {
      switch (bar)
	{
	case 1:
	  if (com != 0)
 	    return align;
	  *(char *) 0 = 0;
	}
    }
}

void
testTortureExecute (void)
{
  f (0, 1, 1);
  return;
}

