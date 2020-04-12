/*
   pr77767.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR c/77767 */

#if 0 // Enable when SDCC allows non-constant expression in array parms
void
foo (int a, int b[a++], int c, int d[c++])
{
  if (a != 2 || c != 2)
    ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
#if 0
  int e[10];
  foo (1, e, 1, e);
  return;
#endif
}
