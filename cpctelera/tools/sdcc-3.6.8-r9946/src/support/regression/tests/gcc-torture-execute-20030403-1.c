/*
   20030403-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* The non-destructive folder was always emitting >= when folding
   comparisons to signed_max+1.  */

#include <limits.h>

void
testTortureExecute (void)
{
  unsigned long count = 8;

  if (count > INT_MAX)
    ASSERT (0);

  return;
}

