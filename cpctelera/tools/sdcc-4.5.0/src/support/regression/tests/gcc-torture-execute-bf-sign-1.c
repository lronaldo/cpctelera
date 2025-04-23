/*
bf-sign-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

void
testTortureExecute (void)
{
  struct  {
    signed int s:3;
    unsigned int u:3;
    int i:3;
  } x = {-1, -1, -1};

  if (x.u != 7)
    ASSERT (0);
  if (x.s != - 1)
    ASSERT (0);

  if (x.i != -1 && x.i != 7)
    ASSERT (0);

  return;
}
