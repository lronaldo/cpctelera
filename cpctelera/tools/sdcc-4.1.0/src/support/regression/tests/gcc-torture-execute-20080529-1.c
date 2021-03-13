/*
   20080529-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/36362 */
#if !(defined (__GNUC__) && __GNUC__ < 5)
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk14)
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
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk14)
  if (test (1.0f) != 0)
    ASSERT (0);
#endif
  return;
#endif
}

