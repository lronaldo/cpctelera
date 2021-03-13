/*
va-arg-2.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85
#pragma disable_warning 196
#endif

#include <string.h>

/* The purpose of this test is to catch edge cases when arguments are passed
   in regs and on the stack.  We test 16 cases, trying to catch multiple
   targets (some use 3 regs for argument passing, some use 12, etc.).
   We test both the arguments and the `lastarg' (the argument to va_start).  */

#include <stdarg.h>

int
to_hex (unsigned int a)
{
  static char hex[] = "0123456789abcdef";

  if (a > 15)
    ASSERT(0);
  return hex[a];
}

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
void
f0 (char* format, ...)
{
  va_list ap;

  va_start (ap, format);
  if (strlen (format) != 16 - 0)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f1 (int a1, char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 1)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f2 (int a1, int a2, char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 2)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f3 (int a1, int a2, int a3, char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 3)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f4 (int a1, int a2, int a3, int a4, char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 4)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f5 (int a1, int a2, int a3, int a4, int a5,
    char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 5)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f6 (int a1, int a2, int a3, int a4, int a5,
    int a6,
    char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 6)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f7 (int a1, int a2, int a3, int a4, int a5,
    int a6, int a7,
    char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 7)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f8 (int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8,
    char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 8)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f9 (int a1, int a2, int a3, int a4, int a5,
     int a6, int a7, int a8, int a9,
     char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 9)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f10 (int a1, int a2, int a3, int a4, int a5,
     int a6, int a7, int a8, int a9, int a10,
     char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 10)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f11 (int a1, int a2, int a3, int a4, int a5,
     int a6, int a7, int a8, int a9, int a10,
     int a11,
     char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 11)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f12 (int a1, int a2, int a3, int a4, int a5,
     int a6, int a7, int a8, int a9, int a10,
     int a11, int a12,
     char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 12)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f13 (int a1, int a2, int a3, int a4, int a5,
     int a6, int a7, int a8, int a9, int a10,
     int a11, int a12, int a13,
     char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 13)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f14 (int a1, int a2, int a3, int a4, int a5,
     int a6, int a7, int a8, int a9, int a10,
     int a11, int a12, int a13, int a14,
     char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 14)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}

void
f15 (int a1, int a2, int a3, int a4, int a5,
     int a6, int a7, int a8, int a9, int a10,
     int a11, int a12, int a13, int a14, int a15,
     char* format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (strlen (format) != 16 - 15)
    ASSERT(0);
  while (*format)
    if (*format++ != to_hex (va_arg (ap, int)))
      ASSERT(0);
  va_end(ap);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  char *f = "0123456789abcdef";

  f0 (f+0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  f1 (0, f+1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  f2 (0, 1, f+2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  f3 (0, 1, 2, f+3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  f4 (0, 1, 2, 3, f+4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  f5 (0, 1, 2, 3, 4, f+5, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  f6 (0, 1, 2, 3, 4, 5, f+6, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  f7 (0, 1, 2, 3, 4, 5, 6, f+7, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  f8 (0, 1, 2, 3, 4, 5, 6, 7, f+8, 8, 9, 10, 11, 12, 13, 14, 15);
  f9 (0, 1, 2, 3, 4, 5, 6, 7, 8, f+9, 9, 10, 11, 12, 13, 14, 15);
  f10 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, f+10, 10, 11, 12, 13, 14, 15);
  f11 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, f+11, 11, 12, 13, 14, 15);
  f12 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, f+12, 12, 13, 14, 15);
  f13 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, f+13, 13, 14, 15);
  f14 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, f+14, 14, 15);
  f15 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, f+15, 15);
#endif
  return;
}
