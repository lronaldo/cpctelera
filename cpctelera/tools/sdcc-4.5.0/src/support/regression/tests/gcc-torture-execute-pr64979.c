/*
   pr64979.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

/* PR target/64979 */

#include <stdarg.h>

#ifndef __SDCC_pdk14 // Lack of memory
void
bar (int x, va_list *ap)
{
  (void)x;
  if (ap)
    {
      int i;
      for (i = 0; i < 10; i++)
        ASSERT (i == va_arg (*ap, int));
      ASSERT (va_arg (*ap, double) == 0.5);
    }
}

void
foo (int x, ...)
{
  va_list ap;
  int n;

  va_start (ap, x);
  n = va_arg (ap, int);
  bar (x, (va_list *) ((n == 0) ? ((void *) 0) : &ap));
  va_end (ap);
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  foo (100, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0.5);
  return;
#endif
}
