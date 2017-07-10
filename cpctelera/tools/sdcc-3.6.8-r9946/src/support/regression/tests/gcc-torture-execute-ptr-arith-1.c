/*
   ptr-arith-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

const char *
f (const char *s, unsigned int i)
{
  return &s[i + 3 - 1];
}

void
testTortureExecute (void)
{
  const char *str = "abcdefghijkl";
  const char *x2 = f (str, 12);
  if (str + 14 != x2)
    ASSERT (0);
  return;
}

