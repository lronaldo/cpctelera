/*
   pr65215-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

/* PR tree-optimization/65215 */

static inline unsigned int
foo (unsigned int x)
{
  return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
}

unsigned long long
bar (unsigned long long *x)
{
  return ((unsigned long long) foo (*x) << 32) | foo (*x >> 32);
}

void
testTortureExecute (void)
{
  if (CHAR_BIT != 8 || sizeof (unsigned int) != 4 || sizeof (unsigned long long) != 8)
    return;
  unsigned long long l = foo (0xfeedbea8U) | ((unsigned long long) foo (0xdeadbeefU) << 32);
  if (bar (&l) != 0xfeedbea8deadbeefULL)
    ASSERT (0);
  return;
}
