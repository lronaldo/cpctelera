/*
   func-ptr-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93 // Using float for double.
#endif

static double f (float a);
static double (*fp) (float a);

void
testTortureExecute (void)
{
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
  fp = f;
  if (fp ((float) 1) != 1.0)
    ASSERT (0);
  return;
#endif
}

static double
f (float a)
{
  return a;
}
