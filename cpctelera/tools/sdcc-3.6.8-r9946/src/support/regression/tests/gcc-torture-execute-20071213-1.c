/*
   20071213-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/34281 */

#include <stdarg.h>

void
h (int x, va_list ap)
{
  switch (x)
    {
    case 1:
      if (va_arg (ap, int) != 3 || va_arg (ap, int) != 4)
	ASSERT (0);
      return;
    case 5:
      if (va_arg (ap, int) != 9 || va_arg (ap, int) != 10)
	ASSERT (0);
      return;
    default:
      ASSERT (0);
    }
}

// Some ports do not yet support long long
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
void
f1 (int i, long long int j, ...)
{
  va_list ap;
  va_start (ap, j);
  h (i, ap);
  if (i != 1 || j != 2)
    ASSERT (0);
  va_end (ap);
}

void
f2 (int i, int j, int k, long long int l, ...)
{
  va_list ap;
  va_start (ap, l);
  h (i, ap);
  if (i != 5 || j != 6 || k != 7 || l != 8)
    ASSERT (0);
  va_end (ap);
}
#endif

void
testTortureExecute (void)
{
// Some ports do not yet support long long
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  f1 (1, 2, 3, 4);
  f2 (5, 6, 7, 8, 9, 10);
#endif
  return;
}
