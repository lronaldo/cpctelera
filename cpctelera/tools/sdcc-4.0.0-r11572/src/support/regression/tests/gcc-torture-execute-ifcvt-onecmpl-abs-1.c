/*
   ifcvt-onecmpl-abs-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int foo(int n)
{
  if (n < 0)
    n = ~n;

  return n;
}

void
testTortureExecute (void)
{
  if (foo (-1) != 0)
    ASSERT (0);

  return;
}

