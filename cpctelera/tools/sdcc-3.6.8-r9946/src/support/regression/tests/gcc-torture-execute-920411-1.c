/*
   920411-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

long f (char *w)
{
  long k, i, c = 0, x;
  char *p = (char*) &x;
  for (i = 0; i < 1; i++)
    {
      for (k = 0; k < sizeof (long); k++)
	p[k] = w[k];
      c += x;
    }
  return c;
}

void
testTortureExecute (void)
{
  int i;
  char a[sizeof (long)];

  for (i = sizeof (long); --i >= 0;) a[i] = ' ';
  if (f (a) != ~0UL / (unsigned char) ~0 * ' ')
    ASSERT (0);
  return;
}

