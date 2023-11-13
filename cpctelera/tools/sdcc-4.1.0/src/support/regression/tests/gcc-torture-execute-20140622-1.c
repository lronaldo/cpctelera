/*
   20140622-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned p;

long
test (unsigned a)
{
  return (long)(p + a) - (long)p;
}

void
testTortureExecute (void)
{
  p = (unsigned) -2;
  if (test (0) != 0)
    ASSERT (0);
  if (test (1) != 1)
    ASSERT (0);
  if (test (2) != -(long)(unsigned)-2)
    ASSERT (0);
  p = (unsigned) -1;
  if (test (0) != 0)
    ASSERT (0);
  if (test (1) != -(long)(unsigned)-1)
    ASSERT (0);
  if (test (2) != -(long)(unsigned)-2)
    ASSERT (0);
}
