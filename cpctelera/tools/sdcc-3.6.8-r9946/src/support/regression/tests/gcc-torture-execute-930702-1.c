/*
   930702-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_ds390) && !defined(__SDCC_mcs51)
int fp (double a, int b)
{
  if (a != 33 || b != 11)
    ASSERT (0);
  return (0);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_ds390) && !defined(__SDCC_mcs51)
  int (*f) (double, int) = fp;

  fp (33, 11);
  f (33, 11);
  return;
#endif
}

