/*
   pr15262.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR middle-end/27260 */

#include <string.h>

char buf[65];

void
foo (int x)
{
  memset (buf, x != 2 ? 1 : 0, 64);
}

void
testTortureExecute (void)
{
  int i;
  buf[64] = 2;
  for (i = 0; i < 64; i++)
    if (buf[i] != 0)
      ASSERT (0);
  foo (0);
  for (i = 0; i < 64; i++)
    if (buf[i] != 1)
      ASSERT (0);
  foo (2);
  for (i = 0; i < 64; i++)
    if (buf[i] != 0)
      ASSERT (0);
  if (buf[64] != 2)
    ASSERT (0);
  return;
}

