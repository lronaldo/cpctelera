/*
   pr79286.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>

int a = 0, c = 0;
#if 0 // Enable when SDCC supports this style of init 
static int d[][8] = {};
#endif
void
testTortureExecute (void)
{
#if 0
  int e;
  for (int b = 0; b < 4; b++)
    {
      printf ("%d\n", b, e);
      while (a && c++)
	e = d[300000000000000000][0];
    }

  return;
#endif
}

#if !defined(PORT_HOST)
int putchar (int c)
{
  c;
}
#endif

