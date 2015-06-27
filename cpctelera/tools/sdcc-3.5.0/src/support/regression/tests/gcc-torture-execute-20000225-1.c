/*
   20000225-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
    int nResult;
    int b=0;
    int i = -1;

    do
    {
     if (b!=0) {
       ASSERT (0);
       nResult=1;
     } else {
      nResult=0;
     }
     i++;
     b=(i+2)*4;
    } while (i < 0);
    return;
}

