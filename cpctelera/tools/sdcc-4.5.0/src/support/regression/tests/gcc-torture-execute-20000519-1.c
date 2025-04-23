/*
   20000519-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdarg.h>

int
bar (int a, va_list ap)
{
  int b;

  do
    b = va_arg (ap, int);
  while (b > 10);

  return a + b;
}

int
foo (int a, ...)
{
  va_list ap;

  va_start (ap, a);
  return bar (a, ap);
}

void
testTortureExecute (void)
{
  if (foo (1, 2, 3) != 3)
    ASSERT (0);
  return;
}

