/*
   pr78378.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#ifndef __SDCC_pdk14 // Lack of memory
/* PR rtl-optimization/78378 */
unsigned long long
foo (unsigned long long x)
{
  x <<= 41;
  x /= 232;
  return 1 + (unsigned short) x;
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  unsigned long long x = foo (1);
  if (x != 0x2c24)
    ASSERT(0);
  return;
#endif
}
