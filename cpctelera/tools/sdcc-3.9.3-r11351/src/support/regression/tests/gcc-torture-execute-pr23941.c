/*
   pr15262.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

#include <float.h>

extern void abort (void);
double d = FLT_MIN / 2.0;

void
testTortureExecute (void)
{
  double x = FLT_MIN / 2.0;
  if (x != d)
    ASSERT (0);
  return;
}

