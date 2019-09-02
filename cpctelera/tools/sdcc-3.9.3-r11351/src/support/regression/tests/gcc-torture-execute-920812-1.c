/*
   920812-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef int t;
f(t y){switch(y){case 1:return 1;}return 0;}
void
testTortureExecute (void){if(f((t)1)!=1)ASSERT(0);return;}

