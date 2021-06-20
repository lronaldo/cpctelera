/*
   20021010-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

int
sub ()
{
  int dummy = 0, a = 16;

  if (a / INT_MAX / 16 == 0)
    return 0;
  else
    return a / INT_MAX / 16;
}

void
testTortureExecute (void)
{
  if (sub () != 0)
    ASSERT (0);

  return;
}
