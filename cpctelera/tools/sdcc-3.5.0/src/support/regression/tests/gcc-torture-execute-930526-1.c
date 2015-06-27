/*
   930526-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0
inline
void f (int x)
{
  int *(p[25]);
  int m[25*7];
  int i;

  for (i = 0; i < 25; i++)
    p[i] = m + x*i;

  p[1][0] = 0;
}
#endif

void
testTortureExecute (void)
{
#if 0
  f (7);
  return;
#endif
}

