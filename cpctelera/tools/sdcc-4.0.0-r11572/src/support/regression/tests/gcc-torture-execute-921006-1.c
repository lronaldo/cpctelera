/*
   921006-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

/* REPRODUCED:RUN:SIGNAL MACHINE:i386 OPTIONS:-O */
void
testTortureExecute (void)
{
if(strcmp("X","")<0)ASSERT(0);
return;
}

