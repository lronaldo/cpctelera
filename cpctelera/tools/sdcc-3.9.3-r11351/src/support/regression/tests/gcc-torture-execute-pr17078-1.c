/*
   pr17078-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void test(int *ptr)
{
  int i = 1;
  goto useless;
  if (0)
    {
      useless:
      i = 0;
    }
  else
    i = 1;
  *ptr = i;
}

void
testTortureExecute (void)
{
  int i = 1;
  test(&i);
  if (i)
    ASSERT (0);
  return;
}

