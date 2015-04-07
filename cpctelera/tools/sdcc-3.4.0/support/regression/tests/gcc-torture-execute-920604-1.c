/*
   920604-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long mod!
#if 0
long long
mod (a, b)
     long long a, b;
{
  return a % b;
}
#endif

void
testTortureExecute (void)
{
#if 0
  mod (1LL, 2LL);
  exit (0);
#endif
}

