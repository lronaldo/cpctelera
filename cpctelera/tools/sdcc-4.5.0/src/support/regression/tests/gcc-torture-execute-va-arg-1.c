/*
va-arg-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdarg.h>

typedef unsigned long L;
void
f (L p0, L p1, L p2, L p3, L p4, L p5, L p6, L p7, L p8, ...)
{
  va_list select;

  (void)p0;
  (void)p1;
  (void)p2;
  (void)p3;
  (void)p4;
  (void)p5;
  (void)p6;
  (void)p7;

  va_start (select, p8);

  ASSERT(va_arg (select, L) == 10);
  ASSERT(va_arg (select, L) == 11);
  ASSERT(va_arg (select, L) == 0);

  va_end (select);
}

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  f (1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L, 10L, 11L, 0L);
  return;
#endif
}
