/*
   921218-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f()
{
  long l2;
  unsigned short us;
  unsigned long ul;
  short s2;

  ul = us = l2 = s2 = -1;
  return ul;
}

void
testTortureExecute (void)
{
  if (f()!=(unsigned short)-1)
    ASSERT(0);
  return;
}

