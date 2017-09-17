/*
   941021-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

double glob_dbl;

void f (double *pdbl, double value)
{
  if (pdbl == 0)
    pdbl = &glob_dbl;

  *pdbl = value;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && defined (__GNUC_MINOR__) && (__GNUC__ < 5))
  f ((void *) 0, 55.1);

  if (glob_dbl != 55.1)
    ASSERT (0);
  return;
#endif
}

