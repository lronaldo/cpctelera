/*
   930622-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Todo: Enable when sdcc supports long double and long long!
#if 0
long double
ll_to_ld (long long n)
{
  return n;
}

long long
ld_to_ll (long double n)
{
  return n;
}
#endif

void
testTortureExecute (void)
{
#if 0
  long long n;

  if (ll_to_ld (10LL) != 10.0)
    ASSERT (0);

  if (ld_to_ll (10.0) != 10)
    ASSERT (0);

  return;
#endif
}

