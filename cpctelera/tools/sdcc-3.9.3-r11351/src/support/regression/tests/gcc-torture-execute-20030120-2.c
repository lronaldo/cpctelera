/*
   20030120-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR 8848 */

int foo(int status)
{
  int s = 0;
  if (status == 1) s=1;
  if (status == 3) s=3;
  if (status == 4) s=4;
  return s;
}

void
testTortureExecute (void)
{
  if (foo (3) != 3)
    ASSERT (0);
  return;
}

