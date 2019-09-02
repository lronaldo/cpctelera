/*
   pr62151.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/62151 */

int a, c, d, e, f, g, h, i;
short b;

int
fn1 ()
{
  b = 0;
  for (;;)
    {
      int j[2];
      j[f] = 0;
      if (h)
	d = 0;
      else
	{
	  for (; f; f++)
	    ;
	  for (a = 0; a < 1; a++)
	    for (;;)
	      {
		i = b & ((b ^ 1) & 83647) ? b : b - 1;
		g = 1 ? i : 0;
		e = j[0];
		if (c)
		  break;
		return 0;
	      }
	}
    }
}

void
testTortureExecute (void)
{
  fn1 ();
  if (g != -1)
    ASSERT (0);
}
