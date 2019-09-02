/*
   921123-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

f(short *p)
{
  short x = *p;
  return (--x < 0);
}

void
testTortureExecute (void)
{
  short x = -10;
  if (!f(&x))
    ASSERT(0);
  return;
}

