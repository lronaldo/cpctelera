/*
packed-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

short x1 = 17;

struct
{
  short i ;
} t;

f ()
{
  t.i = x1;
  if (t.i != 17)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  f ();
  return;
}
