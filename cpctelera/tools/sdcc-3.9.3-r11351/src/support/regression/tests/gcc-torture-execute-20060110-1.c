/*
   20060110-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
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
#if !defined (__SDCC_mcs51)
  if (f (a) != b)
    ASSERT (0);
  return;
#endif
}
