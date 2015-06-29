/*
   980526-3.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int compare(unsigned int x, unsigned int y)
{
   if (x==y)
     return 0;
   else
     return 1;
}
 
void
testTortureExecute (void)
{
 unsigned int i, j, k, l;
 i = 5; j = 2; k=0; l=2;
 if (compare(5%(~(unsigned) 2), i%~j) 
     || compare(0, k%~l))
    ASSERT(0);
 else
    return;
}

