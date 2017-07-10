/*
   921029-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long in these ports!
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
typedef unsigned long long ULL;
ULL back;
ULL hpart, lpart;
ULL
build(long h, long l)
{
  hpart = h;
  hpart <<= 32;
  lpart = l;
  lpart &= 0xFFFFFFFFLL;
  back = hpart | lpart;
  return back;
}
#endif

void
testTortureExecute (void)
{
#if !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
  if (build(0, 1) != 0x0000000000000001LL)
    ASSERT(0);
  if (build(0, 0) != 0x0000000000000000LL)
    ASSERT(0);
  if (build(0, 0xFFFFFFFF) != 0x00000000FFFFFFFFLL)
    ASSERT(0);
  if (build(0, 0xFFFFFFFE) != 0x00000000FFFFFFFELL)
    ASSERT(0);
  if (build(1, 1) != 0x0000000100000001LL)
    ASSERT(0);
  if (build(1, 0) != 0x0000000100000000LL)
    ASSERT(0);
  if (build(1, 0xFFFFFFFF) != 0x00000001FFFFFFFFLL)
    ASSERT(0);
  if (build(1, 0xFFFFFFFE) != 0x00000001FFFFFFFELL)
    ASSERT(0);
  if (build(0xFFFFFFFF, 1) != 0xFFFFFFFF00000001LL)
    ASSERT(0);
  if (build(0xFFFFFFFF, 0) != 0xFFFFFFFF00000000LL)
    ASSERT(0);
  if (build(0xFFFFFFFF, 0xFFFFFFFF) != 0xFFFFFFFFFFFFFFFFLL)
    ASSERT(0);
  if (build(0xFFFFFFFF, 0xFFFFFFFE) != 0xFFFFFFFFFFFFFFFELL)
    ASSERT(0);
  return;
#endif
}

