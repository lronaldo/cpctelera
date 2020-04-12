/*
   20150611-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int a, c, d;
short b;

void
testTortureExecute (void)
{
  int e[1];
  for (; b < 2; b++)
    {
      a = 0;
      if (b == 28378)
        a = e[b];
      if (!(d || b))
        for (; c;)
          ;
    }
}
