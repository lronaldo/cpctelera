/*
   921019-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

void
testTortureExecute (void)
{
  double x,y=0.5;
  x=y/0.2;
  if(x!=x)
    ASSERT(0);
  return;
}

