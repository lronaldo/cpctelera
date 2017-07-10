/*
   20060110-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when long long comes to these ports!
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
long long 
f (long long a) 
{ 
  return (a << 32) >> 32; 
} 
long long a = 0x1234567876543210LL;
long long b = (0x1234567876543210LL << 32) >> 32;
#endif

void
testTortureExecute (void)
{
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
// TODO: Enable when sdcc supports long long constants!
  if (f (a) != b)
    ASSERT (0);
  return;
#endif
}
