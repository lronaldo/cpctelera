/*
va-arg-14.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdarg.h>

#pragma disable_warning 84	// suppress 'auto' variable may be used before initialization

va_list global;

void vat(va_list param, ...)
{
  va_list local;

  va_start (local, param);
  va_copy (global, local);
  va_copy (param, local);
  ASSERT (va_arg (local, int) == 1);
  va_end (local);
  ASSERT (va_arg (global, int) == 1);
  va_end (global);
  ASSERT (va_arg (param, int) == 1);
  va_end (param);

  va_start (param, param);
  va_start (global, param);
  va_copy (local, param);
  ASSERT (va_arg (local, int) == 1);
  va_end (local);
  va_copy (local, global);
  ASSERT (va_arg (local, int) == 1);
  va_end (local);
  ASSERT (va_arg (global, int) == 1);
  va_end (global);
  ASSERT (va_arg (param, int) == 1);
  va_end (param);
}

void
testTortureExecute (void)
{
  va_list t; /* since the va_list type is undefined in C, 't' cannot be initialized */
  vat (t, 1);
  return;
}
