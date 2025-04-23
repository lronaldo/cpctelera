/*
   930123-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
f(int *x)
{
  *x = 0;
}

void
testTortureExecute (void)
{
  int s, c, x;
  char a[] = "c";

  f(&s);
  a[c = 0] = s == 0 ? (x=1, 'a') : (x=2, 'b');
  ASSERT(a[c] == 'a');
  return;
}

