/*
   20010604-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdbool.h>

int func (int a, int b, int c, _Bool d, _Bool e, _Bool f, char g)
{
  if (g != 1 || d != true || e != true || f != true) ASSERT (0);
  return a + b + c;
}

void
testTortureExecute (void)
{
  if (func (1, 2, -3, true, true, true, '\001'))
    ASSERT (0);
  return;
}

