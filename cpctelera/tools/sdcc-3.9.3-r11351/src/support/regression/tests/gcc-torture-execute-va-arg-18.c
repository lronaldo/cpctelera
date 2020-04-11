/*
va-arg-18.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85
#pragma disable_warning 93
#endif

#include <stdarg.h>

typedef double L;
void f (L p0, L p1, L p2, L p3, L p4, L p5, L p6, L p7, L p8, ...)
{
  va_list select;

  va_start (select, p8);

  if (va_arg (select, int) != 10)
    ASSERT(0);
  if (va_arg (select, int) != 11)
    ASSERT(0);
  if (va_arg (select, int) != 12)
    ASSERT(0);

  va_end (select);
}

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  f (1., 2., 3., 4., 5., 6., 7., 8., 9., 10, 11, 12);
  return;
#endif
}
