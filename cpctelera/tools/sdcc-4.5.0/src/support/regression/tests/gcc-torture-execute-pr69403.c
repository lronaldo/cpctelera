/*
   pr69403.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/69403.  */

int a, b, c;

int
fn1 ()
{
  if ((b | (a != (a & c))) == 1)
    ASSERT(0);
  return 0;
}

void
testTortureExecute (void)
{
  a = 5;
  c = 1;
  b = 6;
  fn1 ();
  return;
}

