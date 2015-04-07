/*
   stdarg-4.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 93 // Using float for double.

#include <stdarg.h>

long x, y;

static inline void
f1i (va_list ap)
{
  x = va_arg (ap, double);
  x += va_arg (ap, long);
  x += va_arg (ap, double);
}

void
f1 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  f1i (ap);
  va_end (ap);
}

static inline void
f2i (va_list ap)
{
  y = va_arg (ap, int);
  y += va_arg (ap, long);
  y += va_arg (ap, double);
  f1i (ap);
}

void
f2 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  f2i (ap);
  va_end (ap);
}

long
f3h (int i, long arg0, long arg1, long arg2, long arg3)
{
  return i + arg0 + arg1 + arg2 + arg3;
}

long
f3 (int i, ...)
{
  long t, arg0, arg1, arg2, arg3;
  va_list ap;

  va_start (ap, i);
  switch (i)
    {
    case 0:
      t = f3h (i, 0, 0, 0, 0);
      break;
    case 1:
      arg0 = va_arg (ap, long);
      t = f3h (i, arg0, 0, 0, 0);
      break;
    case 2:
      arg0 = va_arg (ap, long);
      arg1 = va_arg (ap, long);
      t = f3h (i, arg0, arg1, 0, 0);
      break;
    case 3:
      arg0 = va_arg (ap, long);
      arg1 = va_arg (ap, long);
      arg2 = va_arg (ap, long);
      t = f3h (i, arg0, arg1, arg2, 0);
      break;
    case 4:
      arg0 = va_arg (ap, long);
      arg1 = va_arg (ap, long);
      arg2 = va_arg (ap, long);
      arg3 = va_arg (ap, long);
      t = f3h (i, arg0, arg1, arg2, arg3);
      break;
    default:
      t = 0;
      ASSERT (0);
    }
  va_end (ap);

  return t;
}

void
f4 (int i, ...)
{
  va_list ap;

  va_start (ap, i);
  switch (i)
    {
    case 4:
      y = va_arg (ap, double);
      break;
    case 5:
      y = va_arg (ap, double);
      y += va_arg (ap, double);
      break;
    default:
      ASSERT (0);
    }
  f1i (ap);
  va_end (ap);
}


void
testTortureExecute (void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pic16)
  f1 (3, 16.0, 128L, 32.0);
  if (x != 176L)
    ASSERT (0);
  f2 (6, 5, 7L, 18.0, 19.0, 17L, 64.0);
  if (x != 100L || y != 30L)
    ASSERT (0);
  if (f3 (0) != 0)
    ASSERT (0);
  if (f3 (1, 18L) != 19L)
    ASSERT (0);
  if (f3 (2, 18L, 100L) != 120L)
    ASSERT (0);
  if (f3 (3, 18L, 100L, 300L) != 421L)
    ASSERT (0);
  if (f3 (4, 18L, 71L, 64L, 86L) != 243L)
    ASSERT (0);
  f4 (4, 6.0, 9.0, 16L, 18.0);
  if (x != 43L || y != 6L)
    ASSERT (0);
  f4 (5, 7.0, 21.0, 1.0, 17L, 126.0);

// Fails on z80 and related
  if (x != 144L || y != 28L)
    ASSERT (0);
#endif
  return;
}

