/*
   pr49281.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/49281 */

extern void abort (void);

int
foo (int x)
{
  return (x << 2) | 4;
}

int
bar (int x)
{
  return (x << 2) | 3;
}

void
testTortureExecute (void)
{
  if (foo (43) != 172 || foo (1) != 4 || foo (2) != 12)
    ASSERT (0);
  if (bar (43) != 175 || bar (1) != 7 || bar (2) != 11)
    ASSERT (0);
  return;
}

