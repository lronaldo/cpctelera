/*
   20030209-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

#ifndef __SDCC_mcs51
double x[10][10]; /* Was [100][100] originally, while the 9 below was 99, but that was too big for sdcc targets. */
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_mcs51
  int i;

  i = 9;
  x[i][0] = 42;
  if (x[9][0] != 42)
    ASSERT (0);
  return;
#endif

}

