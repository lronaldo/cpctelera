/*
va-arg-16.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 93
#endif

#include <stdarg.h>

#ifndef __SDCC_pdk14 // Lack of memory
typedef double TYPE;

void vafunction (TYPE dummy1, TYPE dummy2, ...)
{
  va_list ap;

  va_start(ap, dummy2);
  if (dummy1 != 888.)
    ASSERT(0);
  if (dummy2 != 999.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 1.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 2.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 3.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 4.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 5.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 6.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 7.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 8.)
    ASSERT(0);
  if (va_arg (ap, TYPE) != 9.)
    ASSERT(0);
  va_end(ap);
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  vafunction( 888., 999., 1., 2., 3., 4., 5., 6., 7., 8., 9. );
  return;
#endif
}
