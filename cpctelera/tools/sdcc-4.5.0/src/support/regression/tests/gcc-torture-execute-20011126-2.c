/*
   20011126-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Problem originally visible on ia64.

   There is a partial redundancy of "in + 1" that makes GCSE want to
   transform the final while loop to 

     p = in + 1;
     tmp = p;
     ...
     goto start;
   top:
     tmp = tmp + 1;
   start:
     in = tmp;
     if (in < p) goto top;

   We miscalculate the number of loop iterations as (p - tmp) = 0
   instead of (p - in) = 1, which results in overflow in the doloop
   optimization.  */

static const char *
test (const char *in, char *out)
{
  while (1)
    {
      if (*in == 'a')
	{
	  const char *p = in + 1;
	  while (*p == 'x')
	    ++p;
	  if (*p == 'b')
	    return p;
	  while (in < p)
	    *out++ = *in++;
	}
    }
}

void
testTortureExecute (void)
{
  char out[4];
  test ("aab", out);
  return;
}

