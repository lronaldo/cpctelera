/*
   20030909-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void test(int x, int y)
{
  if (x == y)
    ASSERT (0);
}

void foo(int x, int y)
{
  if (x == y)
    goto a;
  else
    {
a:;
      if (x == y)
	goto b;
      else
	{
b:;
	  if (x != y)
	    test (x, y);
	}
    }
}

void
testTortureExecute (void)
{
  foo (0, 0);

  return;
}

