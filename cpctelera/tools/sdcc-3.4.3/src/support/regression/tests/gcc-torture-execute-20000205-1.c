/*
   20000205-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static int f (int a)
{
  if (a == 0)
    return 0;
  do
    if (a & 128)
      return 1;
  while (f (0));
  return 0;
}

void
testTortureExecute (void)
{
  if (f (~128))
    ASSERT (0);
  return;
}

