/*
   20060127-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when long long comes to these ports!
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
void
f (long long a)
{
  if ((a & 0xffffffffLL) != 0)
    ASSERT (0);
}

long long a = 0x1234567800000000LL;
#endif

void
testTortureExecute (void)
{
#if 1 // TODO: Enable when long long literals are supported!
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
  f (a);
  return;
#endif
#endif
}

