/*
divconst-3.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifndef __SDCC_pdk14 // Lack of memory
long long
f (long long x)
{
  return x / 10000000000LL;
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  ASSERT (f (10000000000LL) == 1);
  ASSERT (f (100000000000LL) == 10);
  return;
#endif
}
