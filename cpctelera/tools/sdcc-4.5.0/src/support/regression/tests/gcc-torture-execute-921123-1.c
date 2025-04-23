/*
   921123-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
f(short *p)
{
  short x = *p;
  return (--x < 0);
}

void
testTortureExecute (void)
{
  short x = -10;
  ASSERT(f(&x));
  return;
}

