/*
   950511-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_ds400) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_hc08) && !defined(__SDCC_s08)
#if !defined(__SDCC_stm8) && !defined(__SDCC_z80) && !defined(__SDCC_z180) && !defined(__SDCC_gbz80) && !defined(__SDCC_tlcs90) && !defined(__SDCC_r2k) && !defined(__SDCC_r3ka)

  unsigned long long xx;
  unsigned long long *x = (unsigned long long *) &xx;

  *x = -3;
  *x = *x * *x;
  if (*x != 9)
    ASSERT (0);
  return;

#endif
#endif
}

