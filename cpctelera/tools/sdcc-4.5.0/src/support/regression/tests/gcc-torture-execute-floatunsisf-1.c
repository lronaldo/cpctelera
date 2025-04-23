/*
floatunsisf-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* The fp-bit.c function __floatunsisf had a latent bug where guard bits
   could be lost leading to incorrect rounding.  */
/* Origin: Joseph Myers <joseph@codesourcery.com> */

#if __INT_MAX__ >= 0x7fffffff
volatile unsigned u = 0x80000081;
#else
volatile unsigned long u = 0x80000081;
#endif
volatile float f1, f2;
void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14
  f1 = (float) u;
  f2 = (float) 0x80000081;
  if (f1 != f2)
    ASSERT (0);
  return;
#endif
}
