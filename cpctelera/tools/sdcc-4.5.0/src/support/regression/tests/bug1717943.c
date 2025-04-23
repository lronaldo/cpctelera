/*
   bug1717943c.c
     an error in the detection of loopinvariants,
      will move the foo=0 initialisation out of the loops.
 */

#include <testfwk.h>

char foo, firstcall;

char check(void)
{
  if(!firstcall)
    return 1;

  firstcall=0;
  foo = 42;
  return 0;
}

void bug(void)
{
  while(1) {
    foo = 0;
    while(check())
      if(check())
        return;
  }
}


void
testBug(void)
{
  firstcall = 1;
  bug();
  ASSERT(foo == 0);
}
