/*
packed-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

short x1 = 17;

struct
{
  short i ;
} t;

void
f (void)
{
  t.i = x1;
  ASSERT(t.i == 17);
}

void
testTortureExecute (void)
{
  f ();
  return;
}
