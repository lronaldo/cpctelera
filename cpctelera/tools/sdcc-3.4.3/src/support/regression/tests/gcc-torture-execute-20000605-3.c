/*
   20000605-3.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct F { int x; int y; };

void
testTortureExecute (void)
{
  int timeout = 0;
  int x = 0;
  while (1)
    {
      const struct F i = { x++, };
      if (i.x > 0)
	break;
      if (++timeout > 5)
	goto die;
    }
  return;
 die:
  ASSERT (0);
}

