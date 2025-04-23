/*
   20010114-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Origin: PR c/1540 from Mattias Lampe <lampe@tu-harburg.de>,
   adapted to a testcase by Joseph Myers <jsm28@cam.ac.uk>.
   GCC 2.95.2 fails, CVS GCC of 2001-01-13 passes.  */

void
testTortureExecute (void)
{
  int array1[1] = { 1 };
  int array2[2][1]= { { 1 }, { 0 } };
  if (array1[0] != 1)
    ASSERT (0);
  return;
}

