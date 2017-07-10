/*
   20081112-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

static void foo (int a)
{
  int b = (a - 1) + INT_MIN;

  if (b != INT_MIN)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  foo (1);
  return;
}

