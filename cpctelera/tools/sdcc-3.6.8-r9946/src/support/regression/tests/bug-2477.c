/*
  bug-2477.c
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#pragma disable_warning 85
#endif

#if !defined(__SDCC_ds390) && !defined(__SDCC_ds400) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_hc08) && !defined(__SDCC_s08)
long long ss(void)
{
  return -1;
}
#endif

void
testLongLong (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_ds400) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_hc08) && !defined(__SDCC_s08)
  ASSERT (ss () == -1);
#endif
}

