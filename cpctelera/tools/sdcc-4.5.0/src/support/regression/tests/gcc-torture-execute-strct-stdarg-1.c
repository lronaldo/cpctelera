/*
strct-stdarg-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdarg.h>

struct tiny
{
  char c;
  char d;
  char e;
  char f;
  char g;
};

void
f (int n, ...)
{
  struct tiny x;
  int i;

  va_list ap;
  va_start (ap,n);
  for (i = 0; i < n; i++)
    {
      x = va_arg (ap,struct tiny);
      ASSERT(x.c == i + 10);
      ASSERT(x.d == i + 20);
      ASSERT(x.e == i + 30);
      ASSERT(x.f == i + 40);
      ASSERT(x.g == i + 50);
    }
  {
    long x = va_arg (ap, long);
    ASSERT(x == 123);
  }
  va_end (ap);
}

void
testTortureExecute (void)
{
  struct tiny x[3];
  x[0].c = 10;
  x[1].c = 11;
  x[2].c = 12;
  x[0].d = 20;
  x[1].d = 21;
  x[2].d = 22;
  x[0].e = 30;
  x[1].e = 31;
  x[2].e = 32;
  x[0].f = 40;
  x[1].f = 41;
  x[2].f = 42;
  x[0].g = 50;
  x[1].g = 51;
  x[2].g = 52;
#if 0 // TODO: Enable when bug #3365 (passign array element as struct param) is fixed
  f (3, x[0], x[1], x[2], (long) 123);
#endif
  return;
}
