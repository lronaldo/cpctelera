/*
   960402-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long constants!
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
int
f (signed long long int x)
{
  return x > 0xFFFFFFFFLL || x < -0x80000000LL;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  ASSERT(f (0) == 0);
  return;
#endif
}

