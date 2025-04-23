/*
   930702-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

int fp (double a, int b)
{
  if (a != 33 || b != 11)
    ASSERT (0);
  return (0);
}

void
testTortureExecute (void)
{
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) && !defined(__SDCC_ds390) && !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
  int (*f) (double, int) = fp;

  fp (33, 11);
  f (33, 11);
  return;
#endif
}

