/*
   921218-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f()
{
  return (unsigned char)("\377"[0]);
}

void
testTortureExecute (void)
{
  if (f() != (unsigned char)(0377))
    ASSERT(0);
  return;
}

