/*
   Modified 20060110-?.c from the execute part of the gcc torture suite.
   Volatile input to trigger some peephole rules for STM8
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk14) // Lack of memory
long long 
f (volatile long long a, volatile long long b) 
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
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk14) // Lack of memory
  ASSERT (f (a, b) == c);
  return;
#endif
}

