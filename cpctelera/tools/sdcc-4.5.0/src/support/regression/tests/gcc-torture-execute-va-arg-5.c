/*
va-arg-5.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdarg.h>

void
va_double (int n, ...)
{
  va_list args;

  va_start (args, n);

  ASSERT(va_arg (args, double) == 3.141592);
  ASSERT(va_arg (args, double) == 2.71827);
  ASSERT(va_arg (args, double) == 2.2360679);
  ASSERT(va_arg (args, double) == 2.1474836);

  va_end (args);
}
#if 0 // TODO: Enable when SDCC supports long double!
void
va_long_double (int n, ...)
{
  va_list args;

  va_start (args, n);

  ASSERT(va_arg (args, long double) == 3.141592L);
  ASSERT(va_arg (args, long double) == 2.71827L);
  ASSERT(va_arg (args, long double) == 2.2360679L);
  ASSERT(va_arg (args, long double) == 2.1474836L);

  va_end (args);
}
#endif
void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
#if !(defined (__GNUC__) && __GNUC__ < 12) // Test fails on older GCC on 32-bit systems
  va_double (4, 3.141592, 2.71827, 2.2360679, 2.1474836);
#if 0
  va_long_double (4, 3.141592L, 2.71827L, 2.2360679L, 2.1474836L);
#endif
  return;
#endif
#endif
}
