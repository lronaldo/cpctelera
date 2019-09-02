/*
   pr60822.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/64957 */

int
foo (int b)
{
  return (((b ^ 5) | 1) ^ 5) | 1;
}

int
bar (int b)
{
  return (((b ^ ~5) & ~1) ^ ~5) & ~1;
}

void
testTortureExecute (void)
{
  int i;
  for (i = 0; i < 16; i++)
    if (foo (i) != (i | 1) || bar (i) != (i & ~1))
      ASSERT (0);
  return;
}
