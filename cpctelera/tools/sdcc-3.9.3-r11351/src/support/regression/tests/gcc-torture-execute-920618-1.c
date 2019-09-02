/*
   920618-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void){if(1.17549435e-38F<=1.1)return;ASSERT(0);}

