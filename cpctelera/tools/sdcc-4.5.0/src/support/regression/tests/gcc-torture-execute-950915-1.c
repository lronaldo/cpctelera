/*
   950915-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk14)
long int a = 100000;
long int b = 21475;

long
f ()
{
  return ((long long) a * (long long) b) >> 16;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk14)
  if (f () < 0)
    ASSERT (0);
  return;
#endif
}

