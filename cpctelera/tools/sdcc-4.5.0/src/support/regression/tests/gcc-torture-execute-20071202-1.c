/*
   20071202-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: enable when support for compound literals is implemented!
struct T { int t; int r[8]; };
struct S { int a; int b; int c[6]; struct T d; };

void
foo (struct S *s)
{
  *s = (struct S) { s->b, s->a, { 0, 0, 0, 0, 0, 0 }, s->d };
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct S s = { 6, 12, { 1, 2, 3, 4, 5, 6 },
		 { 7, { 8, 9, 10, 11, 12, 13, 14, 15 } } };
  foo (&s);
  if (s.a != 12 || s.b != 6
      || s.c[0] || s.c[1] || s.c[2] || s.c[3] || s.c[4] || s.c[5])
    ASSERT (0);
  if (s.d.t != 7 || s.d.r[0] != 8 || s.d.r[1] != 9 || s.d.r[2] != 10
      || s.d.r[3] != 11 || s.d.r[4] != 12 || s.d.r[5] != 13
      || s.d.r[6] != 14 || s.d.r[7] != 15)
    ASSERT (0);
  return;
#endif
}
