/*
   20021120-3.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>

unsigned int foo (char *c, unsigned int x, unsigned int y)
{
  register unsigned int z;

  sprintf (c, "%d", x / y);
  z = x + 1;
  return z / (y + 1);
}

void
testTortureExecute (void)
{
  char c[16];

  if (foo (c, ~1U, 4) != (~0U / 5))
    ASSERT (0);
  return;
}
