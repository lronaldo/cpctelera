/*
   20011217-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

void
testTortureExecute (void)
{
  double x = 1.0;
  double y = 2.0;

  if ((y > x--) != 1)
    ASSERT (0);
  return;
}

