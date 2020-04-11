/*
   20001203-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Origin: PR c/410 from Jan Echternach
   <jan.echternach@informatik.uni-rostock.de>,
   adapted to a testcase by Joseph Myers <jsm28@cam.ac.uk>.
*/

static void
foo (void)
{
  struct {
    long a;
    char b[1];
  } x = { 2, { 0 } };
}

void
testTortureExecute (void)
{
  int tmp;
  foo ();
  tmp = 1;
  return;
}

