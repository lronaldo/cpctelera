/*
   20010222-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int a[2] = { 18, 6 };

void
testTortureExecute (void)
{
  int b = (-3 * a[0] -3 * a[1]) / 12;
  if (b != -6)
    ASSERT (0);
  return;
}

