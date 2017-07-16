/*
   921202-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Some ports do not support long long yet.
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
int
f(long long x)
{
  x >>= 8;
  return x & 0xff;
}
#endif

void
testTortureExecute (void)
{
// Some ports do not support long long yet.
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  if (f(0x0123456789ABCDEFLL) != 0xCD)
    ASSERT(0);
  return;
#endif
}

