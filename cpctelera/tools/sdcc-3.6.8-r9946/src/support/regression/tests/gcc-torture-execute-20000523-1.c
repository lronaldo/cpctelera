/*
   20000523-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
// long long seems somewhat broken on MacOSX
#if !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
  long long   x;
  int         n;

  if (sizeof (long long) < 8)
    return;
  
  n = 9;
  x = (((long long) n) << 55) / 0xff; 

  if (x == 0)
    ASSERT (0);

  x = (((long long) 9) << 55) / 0xff;

  if (x == 0)
    ASSERT (0);

  return;
#endif
}

