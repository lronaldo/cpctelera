/*
   pr65418-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR tree-optimization/65418 */

int
foo (int x)
{
  if (x == -216 || x == -132 || x == -218 || x == -146)
     return 1;
  return 0;
}

void
testTortureExecute (void)
{
  volatile int i;
  for (i = -230; i < -120; i++)
    if (foo (i) != (i == -216 || i == -132 || i == -218 || i == -146))
      ASSERT (0);
  return;
}
