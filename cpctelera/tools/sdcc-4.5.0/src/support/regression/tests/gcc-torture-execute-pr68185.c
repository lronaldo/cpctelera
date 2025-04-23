/*
   pr68158.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* { dg-skip-if "ptxas crashes or executes incorrectly" { nvptx-*-* } { "-O0" "-Os" } { "" } } Reported 2015-11-20  */

int a, b, d = 1, e, f, o, u, w = 1, z;
short c, q, t;

void
testTortureExecute (void)
{
#if 0 // Enable when SDCC intermingles
  char g;
  for (; d; d--)
    {
      while (o)
	for (; e;)
	  {
	    c = b;
	    int h = o = z;
	    for (; u;)
	      for (; a;)
		;
	  }
      if (t < 1)
	g = w;
      f = g;
      g && (q = 1);
    }

  if (q != 1)
    ASSERT (0);
#endif
  return;
}
