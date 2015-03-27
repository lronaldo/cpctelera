/*
   920409-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
x(void){signed char c=-1;return c<0;}

void
testTortureExecute (void){if(x()!=1)ASSERT(0);return;}

