/*
   961026-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
test (int arg)
{
  if (arg > 0 || arg == 0)
    return 0;
  return -1;
}

void
testTortureExecute (void)
{
  if (test (0) != 0)
    ASSERT (0);
  if (test (-1) != -1)
    ASSERT (0);
  return;
}

