/*
   20031211-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif


struct a { unsigned int bitfield : 1; };

unsigned int x;

void
testTortureExecute (void)
{
  struct a a = {0};
  x = 0xbeef;
  a.bitfield |= x;
  if (a.bitfield != 1)
    ASSERT (0);
  return;
}

