/*
   950503-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  int tmp;
  unsigned long long utmp1, utmp2;

  tmp = 16;

  utmp1 = (~((unsigned long long) 0)) >> tmp;
  utmp2 = (~((unsigned long long) 0)) >> 16;

  if (utmp1 != utmp2)
    ASSERT (0);
  return;
#endif
}

