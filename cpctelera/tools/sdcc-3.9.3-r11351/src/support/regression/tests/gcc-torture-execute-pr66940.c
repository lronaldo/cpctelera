/*
   pr66940.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

long long
foo (long long ival)
{
 if (ival <= 0)
    return -0x7fffffffffffffffL - 1;

 return 0x7fffffffffffffffL;
}

void
testTortureExecute (void)
{
  ASSERT (foo (-1) == (-0x7fffffffffffffffL - 1));
  ASSERT (foo (1) == 0x7fffffffffffffffL);

  ASSERT (foo (-1) != (-0x7fffffff - 1));

  return;
}
