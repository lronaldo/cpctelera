/*
   20021119-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR 8639.  */

int foo (int i)
{
  int r;
  r = (80 - 4 * i) / 20;
  return r;
}
    
void
testTortureExecute (void)
{
  if (foo (1) != 3)
    ASSERT (0);
  return;
}
