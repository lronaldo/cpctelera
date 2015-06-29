/*
   941110-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f (const int x)
{
  int y = 0;
  y = x ? y : -y;
  {
    const int *p = &x;
  }
  return y;
}

void
testTortureExecute (void)
{
  if (f (0))
    ASSERT (0);
  return;
}

