/*
   980929-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void f(int i)
{
  if (i != 1000)
    ASSERT (0);
}


void
testTortureExecute (void)
{
  int n=1000;
  int i;

  f(n);
  for(i=0; i<1; ++i) {
    f(n);
    n=666;
    &n;
  }

  return;
}
