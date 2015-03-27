/*
   920501-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

int s[2];

int
x(int parm)
{
  if(!s[0])
    {
      s[1+s[1]]=s[1];
      return 1;
    }
}

void
testTortureExecute (void){s[0]=s[1]=0;if(x(0)!=1)ASSERT(0);return;}

