/*
   950605-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void f (unsigned char c)
{
  if (c != 0xFF)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  f (-1);
  return;
}

