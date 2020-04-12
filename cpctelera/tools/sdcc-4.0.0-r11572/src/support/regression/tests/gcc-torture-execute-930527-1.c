/*
   930527-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f (unsigned char x)
{
  return (0x50 | (x >> 4)) ^ 0xff;
}

void
testTortureExecute (void)
{
  if (f (0) != 0xaf)
    ASSERT (0);
  return;
}

