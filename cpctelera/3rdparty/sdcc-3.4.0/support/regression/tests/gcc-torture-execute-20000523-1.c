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
#if 0
// Some ports do not support long long yet.
//#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_gbz80)
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

