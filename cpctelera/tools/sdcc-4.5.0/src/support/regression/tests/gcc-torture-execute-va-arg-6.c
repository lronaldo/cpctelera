/*
va-arg-6.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdarg.h>

void
f (int n, ...)
{
  va_list args;

  va_start (args, n);

  ASSERT(va_arg (args, int) == 10);
  ASSERT(va_arg (args, long long) == 10000000000LL);
  ASSERT(va_arg (args, int) == 11);
#if 0 // TODO: Enable when SDCC supports long double!
  ASSERT(va_arg (args, long double) == 3.14L);
  ASSERT(va_arg (args, int) == 12);
  ASSERT(va_arg (args, int) == 13);
  ASSERT(va_arg (args, long long) == 20000000000LL);
  ASSERT(va_arg (args, int) == 14);
  ASSERT(va_arg (args, double) == 2.72);
#endif
  va_end(args);
}

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  f (4, 10, 10000000000LL, 11, 3.14L, 12, 13, 20000000000LL, 14, 2.72);
  return;
#endif
}
