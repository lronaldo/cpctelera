/*
   pr34070-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f(unsigned int x, int n)
{
    return ((int)x) / (1 << n);
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && defined (__GNUC_MINOR__) && (__GNUC__ < 5 && __GNUC_MINOR__ < 3))
  if (f(-1, 1) != 0)
    ASSERT (0);
  return;
#endif 
}

