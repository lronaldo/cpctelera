/*
   20001108-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
long long
signed_poly (long long sum, long x)
{
  sum += (long long) (long) sum * (long long) x;
  return sum;
}

unsigned long long
unsigned_poly (unsigned long long sum, unsigned long x)
{
  sum += (unsigned long long) (unsigned long) sum * (unsigned long long) x;
  return sum;
}
#endif

void
testTortureExecute (void)
{
// Test fails on 32-bit systems
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
#if !defined (__SDCC_gbz80) // bug #2329
  if (signed_poly (2LL, -3) != -4LL)
    ASSERT (0);
  
  if (unsigned_poly (2ULL, 3) != 8ULL)
    ASSERT (0);

  return;
#endif
#endif
}

