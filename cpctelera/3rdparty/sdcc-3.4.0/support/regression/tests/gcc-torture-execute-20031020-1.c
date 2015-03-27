/*
   20031020-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/12654
   The Alpha backend tried to do a >= 1024 as (a - 1024) >= 0, which fails
   for very large negative values.  */
/* Origin: tg@swox.com  */

#include <limits.h>

void
foo (long x)
{
  if (x >= 1024)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  foo (LONG_MIN);
  foo (LONG_MIN + 10000);
  return;
}

