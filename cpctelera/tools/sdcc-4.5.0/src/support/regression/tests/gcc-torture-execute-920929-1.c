/*
   920929-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Todo: Enable when sdcc supports VLA!
#if 0
/* REPRODUCED:RUN:SIGNAL MACHINE:sparc OPTIONS: */
f(int n)
{
int i;
double v[n];
for(i=0;i<n;i++)
v[i]=0;
}
#endif

void
testTortureExecute (void)
{
#if 0
f(100);
return;
#endif
}

