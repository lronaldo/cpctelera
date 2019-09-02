/*
   961122-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long in these ports!
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
long long acc;

void
addhi (short a)
{
  acc += (long long) a << 32;
}

void
subhi (short a)
{
  acc -= (long long) a << 32;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  acc = 0xffff00000000ll;
  addhi (1);
  ASSERT (acc == 0x1000000000000ll);
  subhi (1);
  ASSERT (acc == 0xffff00000000ll);
  return;
#endif
}
