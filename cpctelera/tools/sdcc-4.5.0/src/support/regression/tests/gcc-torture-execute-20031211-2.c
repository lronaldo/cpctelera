/*
   20031211-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct a
{
  unsigned int bitfield : 3;
};

void foo(unsigned int z);

void
testTortureExecute (void)
{
  struct a a;

  a.bitfield = 131;
  foo (a.bitfield);
  return;
}

void foo(unsigned int z)
{
  if (z != 3)
    ASSERT (0);
}

