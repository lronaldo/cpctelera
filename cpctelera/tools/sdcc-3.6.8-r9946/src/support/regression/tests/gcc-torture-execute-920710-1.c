/*
   920710-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
// Enable when sdcc supports double and unsigned long long.
#if 1
  if ((double) 18446744073709551615ULL < 1.84467440737095e+19 ||
      (double) 18446744073709551615ULL > 1.84467440737096e+19)
    ASSERT(0);

  if (16777217L != (float)16777217e0)
    ASSERT(0);

  return;
#endif
}

