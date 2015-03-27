/*
   20040309-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int foo(unsigned short x)
{
  unsigned short y;
  y = x > 32767 ? x - 32768 : 0;
  return y;
}

void
testTortureExecute (void)
{
  if (foo (0) != 0)
    ASSERT (0);
  if (foo (32767) != 0)
    ASSERT (0);
  if (foo (32768) != 0)
    ASSERT (0);
  if (foo (32769) != 1)
    ASSERT (0);
  if (foo (65535) != 32767)
    ASSERT (0);
  return;
}

