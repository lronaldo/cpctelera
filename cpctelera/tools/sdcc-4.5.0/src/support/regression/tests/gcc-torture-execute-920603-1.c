/*
   920603-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
f(int got)
{
  if (got!=0xffff)
    ASSERT(0);
}

void
testTortureExecute (void){signed char c=-1;unsigned u=(unsigned short)c;f(u);return;}

