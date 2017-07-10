/*
   931004-6.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Todo: Enable when sdcc supports struct!
#if 0
#include <stdarg.h>

struct tiny
{
  short c;
  short d;
};

f (int n, ...)
{
  struct tiny x;
  int i;

  va_list ap;
  va_start (ap,n);
  for (i = 0; i < n; i++)
    {
      x = va_arg (ap,struct tiny);
      if (x.c != i + 10)
	ASSERT (0);
      if (x.d != i + 20)
	ASSERT (0);
    }
  {
    long x = va_arg (ap, long);
    if (x != 123)
      ASSERT (0);
  }
  va_end (ap);
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct tiny x[3];
  x[0].c = 10;
  x[1].c = 11;
  x[2].c = 12;
  x[0].d = 20;
  x[1].d = 21;
  x[2].d = 22;
  f (3, x[0], x[1], x[2], (long) 123);
  return;
#endif
}

