/*
   920721-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

long f(short a,short b){return (long)a/b;}
void
testTortureExecute (void){if(f(-32768,-1)!=32768L)ASSERT(0);else return;}
