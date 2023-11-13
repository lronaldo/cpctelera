/*
   pr88693.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR bootstrap/88714 */

struct S { int a, b, c; int *d; };
struct T { int *e, *f, *g; } *t = 0;
int *o = 0;

void bar (int *x, int y, int z, int w)
{
  if (w == -1)
    {
      if (x != 0 || y != 0 || z != 0)
	ASSERT (0);
    }
  else if (w != 0 || x != t->g || y != 0 || z != 12)
    ASSERT (0);
}

void
foo (struct S *x, struct S *y, int *z, int w)
{
  *o = w;
  if (w)
    bar (0, 0, 0, -1);
  x->d = z;
  if (y->d)
    y->c = y->c + y->d[0];
  bar (t->g, 0, y->c, 0);
}

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of RAM
  int a[4] = { 8, 9, 10, 11 };
  struct S s = { 1, 2, 3, &a[0] };
  struct T u = { 0, 0, &a[3] };
  o = &a[2];
  t = &u;
  foo (&s, &s, &a[1], 5);
  if (s.c != 12 || s.d != &a[1])
    ASSERT (0);
  return;
#endif
}
