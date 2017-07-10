/*
   950710-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
struct twelve
{
  int a;
  int b;
  int c;
};

struct pair
{
  int first;
  int second;
};

struct pair
g ()
{
  struct pair p;
  return p;
}

static void
f ()
{
  int i;
  for (i = 0; i < 1; i++)
    {
      int j;
      for (j = 0; j < 1; j++)
	{
	  if (0)
	    {
	      int k;
	      for (k = 0; k < 1; k++)
		{
		  struct pair e = g ();
		}
	    }
	  else
	    {
	      struct twelve a, b;
	      if ((((char *) &b - (char *) &a) < 0
		   ? (-((char *) &b - (char *) &a))
		   : ((char *) &b - (char *) &a))  < sizeof (a))
		ASSERT (0);
	    }
	}
    }
}
#endif

void
testTortureExecute (void)
{
#if 0
  f ();
  return;
#endif
}

