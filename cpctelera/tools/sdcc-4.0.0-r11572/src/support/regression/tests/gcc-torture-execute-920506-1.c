/*
   920506-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int l[]={0,1};

void
testTortureExecute (void){int*p=l;switch(*p++){case 0:return;case 1:break;case 2:break;case 3:case 4:break;}ASSERT(0);}
