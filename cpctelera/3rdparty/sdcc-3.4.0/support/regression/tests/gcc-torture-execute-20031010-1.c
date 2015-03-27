/*
   20031010-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif


/* A reminder to process ops in generate_expr_as_of_bb exactly once.  */

long
foo (long ct, long cf, _Bool p1, _Bool p2, _Bool p3)
{
  long diff;

  diff = ct - cf;

  if (p1)
    {
      if (p2)
	{
	  if (p3)
	    {
	      long tmp = ct;
	      ct = cf;
	      cf = tmp;
	    }
	  diff = ct - cf;
	}

      return diff;
    }

  ASSERT (0);
}

void
testTortureExecute (void)
{
  if (foo(2, 3, 1, 1, 1) == 0)
    ASSERT (0);
  return;
}

