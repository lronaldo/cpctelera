/*
pr83477.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

int yf = 0;

void
pl (int q5, int nd)
{
  unsigned int hp = q5;
  int zx = (q5 == 0) ? hp : (hp / q5);

  yf = ((nd < 2) * zx != 0) ? nd : 0;
}

void
testTortureExecute (void)
{
  pl (1, !yf);
  if (yf != 1)
    ASSERT (0);

  return;
}


