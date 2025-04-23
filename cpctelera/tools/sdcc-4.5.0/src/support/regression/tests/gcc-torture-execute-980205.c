/*
   980205.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

#include <stdarg.h>

void fdouble (double one, ...)
{

  double value;
  va_list ap;

  va_start (ap, one);
  value = va_arg (ap, double);
  va_end (ap);
#ifndef __SDCC_pdk14 // Lack of memory
  if (one != 1.0 || value != 2.0)
    ASSERT (0);
#endif
}

void
testTortureExecute (void)
{
  fdouble (1.0, 2.0);
  return;
}

