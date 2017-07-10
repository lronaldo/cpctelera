/*
   20050131-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Verify that we do not lose side effects on a MOD expression.  */

#include <stdlib.h>
#include <stdio.h>

int
foo (int a)
{
  int x = 0 % a++;
  return a;
}

void
testTortureExecute (void)
{
  if (foo (9) != 10)
    ASSERT (0);
  return;
}
