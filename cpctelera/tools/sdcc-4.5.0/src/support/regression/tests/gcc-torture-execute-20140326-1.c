/*
   20140326-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int a;

void
testTortureExecute (void)
{
  char e[2] = { 0, 0 }, f = 0;
  if (a == 131072)
    f = e[a];
}
