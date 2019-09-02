/*
   951204-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void f (char *x)
{
  *x = 'x';
}

void
testTortureExecute (void)
{
  int i;
  char x = '\0';

  for (i = 0; i < 100; ++i)
    {
      f (&x);
      if (*(const char *) &x != 'x')
	ASSERT (0);
    }
  return;
}

