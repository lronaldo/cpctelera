/*
   pr28651.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

int
foo (unsigned int u)
{
  return (int)(u + 4) < (int)u;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && defined (__GNUC_MINOR__) && (__GNUC__ < 5 && __GNUC_MINOR__ < 1))
  unsigned int u = INT_MAX;

  if (foo (u) == 0)
    ASSERT (0);
  return;
#endif
}

