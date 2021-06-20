/*
   20060110-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when long long comes to these ports!
#if !defined (__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
long long 
f (long long a, long long b) 
{ 
  return ((a + b) << 32) >> 32; 
} 

long long a = 0x1234567876543210LL;
long long b = 0x2345678765432101LL;
long long c = ((0x1234567876543210LL + 0x2345678765432101LL) << 32) >> 32;
#endif

void
testTortureExecute (void)
{
// TODO: Enable when long long literals are supported!
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
  if (f (a, b) != c)
    ASSERT (0);
  return;
#endif
}

