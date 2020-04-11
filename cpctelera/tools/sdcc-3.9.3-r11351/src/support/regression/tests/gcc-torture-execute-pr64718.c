/*
   pr64718.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static int
swap (int x)
{
  return (unsigned short) ((unsigned short) x << 8 | (unsigned short) x >> 8);
}

static int a = 0x1234;

void
testTortureExecute (void)
{
  int b = 0x1234;
  if (swap (a) != 0x3412)
    ASSERT (0);
  if (swap (b) != 0x3412)
    ASSERT (0);
  return;
}
