/*
   20030117-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int foo (int, int, int);
int bar (int, int, int);

void
testTortureExecute (void)
{
  if (foo (5, 10, 21) != 12)
    ASSERT (0);

  if (bar (9, 12, 15) != 150)
    ASSERT (0);

  return;
}

int foo (int x, int y, int z)
{
  return (x + y + z) / 3;
}

int bar (int x, int y, int z)
{
  return foo (x * x, y * y, z * z);
}

