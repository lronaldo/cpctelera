/*
va-arg-17.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#pragma disable_warning 93

#include <stdarg.h>

typedef double TYPE;

void vafunction (char *dummy, ...)
{
  va_list ap;

  va_start(ap, dummy);
#if !defined(__SDCC_pdk14) // Lack of memory
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
#endif
  va_end(ap);
}


void
testTortureExecute (void)
{
  vafunction( "", 1., 2., 3., 4., 5., 6., 7., 8., 9. );
  return;
}
