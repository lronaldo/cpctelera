/*
   941015-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long constants!
#if !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
int
foo1 (long long value)
{
  register const long long constant = 0xc000000080000000LL;

  if (value < constant)
    return 1;
  else
    return 2;
}

int
foo2 (unsigned long long value)
{
  register const unsigned long long constant = 0xc000000080000000LL;

  if (value < constant)
    return 1;
  else
    return 2;
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
  unsigned long long value = 0xc000000000000001LL;
  int x, y;

  x = foo1 (value);
  y = foo2 (value);
  if (x != y || x != 1)
    ASSERT (0);
  return;
#endif
}

