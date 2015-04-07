/*
   20080529-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/36362 */
#if !(defined (__GNUC__) && __GNUC__ < 5)
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
int
test (float c)
{
  return !!c * 7LL == 0;
}
#endif
#endif

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && __GNUC__ < 5)
#if !defined(__SDCC_mcs51) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  if (test (1.0f) != 0)
    ASSERT (0);
#endif
  return;
#endif
}

