/*
   20050111-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Some ports do not yet support long long
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)

/* PR middle-end/19084, rtl-optimization/19348 */

unsigned int
foo (unsigned long long x)
{
  unsigned int u;

  if (x == 0)
    return 0;
  u = (unsigned int) (x >> 32);
  return u;
}

unsigned long long
bar (unsigned short x)
{
  return (unsigned long long) x << 32;
}
#endif

void
testTortureExecute (void)
{
// Some ports do not yet support long long
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08)
  if (sizeof (long long) != 8)
    return;

  if (foo (0) != 0)
    ASSERT (0);

  if (foo (0xffffffffULL) != 0)
    ASSERT (0);
  if (foo (0x25ff00ff00ULL) != 0x25)
    ASSERT (0);
  if (bar (0) != 0)
    ASSERT (0);
  if (bar (0x25) != 0x2500000000ULL)
    ASSERT (0);
  return;
#endif
#endif
}
