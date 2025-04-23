/*
   931004-13.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Todo: Enable when sdcc supports struct!
#if 0
struct tiny
{
  char c;
  char d;
  char e;
  char f;
};

f (int n, struct tiny x, struct tiny y, struct tiny z, long l)
{
  if (x.c != 10)
    ASSERT (0);
  if (x.d != 20)
    ASSERT (0);
  if (x.e != 30)
    ASSERT (0);
  if (x.f != 40)
    ASSERT (0);

  if (y.c != 11)
    ASSERT (0);
  if (y.d != 21)
    ASSERT (0);
  if (y.e != 31)
    ASSERT (0);
  if (y.f != 41)
    ASSERT (0);

  if (z.c != 12)
    ASSERT (0);
  if (z.d != 22)
    ASSERT (0);
  if (z.e != 32)
    ASSERT (0);
  if (z.f != 42)
    ASSERT (0);

  if (l != 123)
    ASSERT (0);
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
  x[0].e = 30;
  x[1].e = 31;
  x[2].e = 32;
  x[0].f = 40;
  x[1].f = 41;
  x[2].f = 42;
  f (3, x[0], x[1], x[2], (long) 123);
  return;
#endif
}

