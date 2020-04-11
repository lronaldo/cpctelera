/*
inst-check.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdarg.h>

#if 0 // TODO: EMable when SDCC supports K&R-style
f(m)
{
  int i,s=0;
  for(i=0;i<m;i++)
    s+=i;
  return s;
}
#endif

void
testTortureExecute (void)
{
  return;
}
