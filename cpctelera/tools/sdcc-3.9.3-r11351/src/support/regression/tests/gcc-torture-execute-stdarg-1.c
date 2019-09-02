/*
stdarg-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 93
#pragma disable_warning 85
#endif

#include <stdarg.h>

int foo_arg, bar_arg;
long x;
double d;
va_list gap;
va_list *pap;

void
foo (int v, va_list ap)
{
  switch (v)
    {
    case 5: foo_arg = va_arg (ap, int); break;
    default: ASSERT (0);
    }
}

#if 0 // TODO: Enable when SDCC support long double!
void
bar (int v)
{
  if (v == 0x4006)
    {
      if (va_arg (gap, double) != 17.0
	  || va_arg (gap, long) != 129L)
	ASSERT (0);
    }
  else if (v == 0x4008)
    {
      if (va_arg (*pap, long long) != 14LL
	  || va_arg (*pap, long double) != 131.0L
	  || va_arg (*pap, int) != 17)
	ASSERT (0);
    }
  bar_arg = v;
}
#endif

void
f0 (int i, ...)
{
}

void
f1 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  va_end (ap);
}
#if 0 // TODO: Enable when SDCC support long double!
void
f2 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  bar (d);
  x = va_arg (ap, long);
  bar (x);
  va_end (ap);
}

void
f3 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  d = va_arg (ap, double);
  va_end (ap);
}

void
f4 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  x = va_arg (ap, double);
  foo (i, ap);
  va_end (ap);
}

void
f5 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  va_copy (gap, ap);
  bar (i);
  va_end (ap);
  va_end (gap);
}

void
f6 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  bar (d);
  va_arg (ap, long);
  va_arg (ap, long);
  x = va_arg (ap, long);
  bar (x);
  va_end (ap);
}

void
f7 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  pap = &ap;
  bar (i);
  va_end (ap);
}

void
f8 (int i, ...)
{
  va_list ap;
  va_start (ap, i);
  pap = &ap;
  bar (i);
  d = va_arg (ap, double);
  va_end (ap);
}
#endif
void
testTortureExecute (void)
{
  f0 (1);
  f1 (2);
  d = 31.0;
#if 0 // TODO: Enable when SDCC support long double!
  f2 (3, 28L);
  if (bar_arg != 28 || x != 28)
    ASSERT (0);
  f3 (4, 131.0);
  if (d != 131.0)
    ASSERT (0);
  f4 (5, 16.0, 128);
  if (x != 16 || foo_arg != 128)
    ASSERT (0);
  f5 (0x4006, 17.0, 129L);
  if (bar_arg != 0x4006)
    v
  f6 (7, 12L, 14L, -31L);
  if (bar_arg != -31)
    ASSERT (0);
  f7 (0x4008, 14LL, 131.0L, 17, 26.0);
  if (bar_arg != 0x4008)
    ASSERT (0);
  f8 (0x4008, 14LL, 131.0L, 17, 27.0);
  if (bar_arg != 0x4008 || d != 27.0)
    ASSERT (0);
#endif
  return;
}
