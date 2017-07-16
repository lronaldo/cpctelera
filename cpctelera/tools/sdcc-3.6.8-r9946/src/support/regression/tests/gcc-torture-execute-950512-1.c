/*
   950512-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long in these ports!
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)

unsigned
f1 (int x)
{
  return ((unsigned) (x != 0) - 3) / 2;
}

unsigned long long
f2 (int x)
{
  return ((unsigned long long) (x != 0) - 3) / 2;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_hc08) && !defined(__SDCC_s08)
  if (f1 (1) != (~(unsigned) 0) >> 1)
    ASSERT (0);
  if (f1 (0) != ((~(unsigned) 0) >> 1) - 1)
    ASSERT (0);
  if (f2 (1) != (~(unsigned long long) 0) >> 1)
    ASSERT (0);
  if (f2 (0) != ((~(unsigned long long) 0) >> 1) - 1)
    ASSERT (0);
  return;
#endif
}

