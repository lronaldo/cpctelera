/*
   pr65418-w.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR tree-optimization/65418 */

int
foo (int x)
{
  if (x == -216 || x == -211 || x == -218 || x == -205 || x == -223)
     return 1;
  return 0;
}

void
testTortureExecute (void)
{
  volatile int i;
  for (i = -230; i < -200; i++)
    if (foo (i) != (i == -216 || i == -211 || i == -218 || i == -205 || i == -223))
      ASSERT (0);
  return;
}
