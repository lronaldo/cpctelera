/*
   pr19606.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Todo: Enable when sdcc supports long long modulo!
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08) && !defined (__SDCC_pic14)
/* PR c/19606
   The C front end used to shorten the type of a division to a type
   that does not preserve the semantics of the original computation.
   Make sure that won't happen.  */

signed char a = -4;

int
foo (void)
{
  return ((unsigned int) (signed int) a) / 2LL;
}

int
bar (void)
{
  return ((unsigned int) (signed int) a) % 5LL;
}
#endif
void
testTortureExecute (void)
{
#if !defined (__SDCC_mcs51) && !defined (__SDCC_ds390) && !defined (__SDCC_hc08) && !defined (__SDCC_s08) && !defined (__SDCC_pic14)
#if !defined (PORT_HOST) // failed in test-host
  int r;

  r = foo ();
  if (r != ((unsigned int) (signed int) (signed char) -4) / 2LL)
    ASSERT (0);

  r = bar ();
  if (r != ((unsigned int) (signed int) (signed char) -4) % 5LL)
    ASSERT (0);

  return;
#endif
#endif
}
