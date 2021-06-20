/*
   950704-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports long long in these ports!
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
int errflag;

long long
f (long long x, long long y)
{
  long long r;

  errflag = 0;
  r = x + y;
  if (x >= 0)
    {
      if ((y < 0) || (r >= 0))
	return r;
    }
  else
    {
      if ((y > 0) || (r < 0))
	return r;
    }
  errflag = 1;
  return 0;
}
#endif

void
testTortureExecute (void)
{
#if !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08)
// TODO: Enable when sdcc supports long long!
  f (0, 0);
  if (errflag)
    ASSERT (0);

  f (1, -1);
  if (errflag)
    ASSERT (0);

  f (-1, 1);
  if (errflag)
    ASSERT (0);

  f (0x8000000000000000LL, 0x8000000000000000LL);
  if (!errflag)
    ASSERT (0);

  f (0x8000000000000000LL, -1LL);
  if (!errflag)
    ASSERT (0);

  f (0x7fffffffffffffffLL, 0x7fffffffffffffffLL);
  if (!errflag)
    ASSERT (0);

  f (0x7fffffffffffffffLL, 1LL);
  if (!errflag)
    ASSERT (0);

  f (0x7fffffffffffffffLL, 0x8000000000000000LL);
  if (errflag)
    ASSERT (0);

  return;
#endif
}

