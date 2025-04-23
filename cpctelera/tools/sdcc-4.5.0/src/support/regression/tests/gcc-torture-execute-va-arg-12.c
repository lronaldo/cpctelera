/*
va-arg-12.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#pragma disable_warning 85
#pragma disable_warning 93

#include <stdarg.h>

/*typedef unsigned long L;*/
typedef double L;
void f (L p0, L p1, L p2, L p3, L p4, L p5, L p6, L p7, L p8, ...)
{
  va_list select;

  va_start (select, p8);

  if (va_arg (select, L) != 10.)
    ASSERT (0);
  if (va_arg (select, L) != 11.)
    ASSERT (0);
  if (va_arg (select, L) != 0.)
    ASSERT (0);

  va_end (select);
}

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  f (1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 0.);
  return;
#endif
}
