/*
   921207-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f(void)
{
  unsigned b = 0;

  if (b > ~0U)
    b = ~0U;

  return b;
}
void
testTortureExecute (void)
{
  if (f()!=0)
    ASSERT(0);
  return;
}

