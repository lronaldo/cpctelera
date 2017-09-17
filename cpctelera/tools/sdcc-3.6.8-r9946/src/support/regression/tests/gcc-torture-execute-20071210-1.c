/*
   20071210-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: Enable when struct can be returned!
/* PR rtl-optimization/34302 */

struct S
{
  int n1, n2, n3, n4;
};

struct S
foo (int x, int y, int z)
{
  if (x != 10 || y != 9 || z != 8)
    abort ();
  struct S s = { 1, 2, 3, 4 };
  return s;
}

void **
bar (void **u, int *v)
{
  void **w = u;
  int *s = v, x, y, z;
  void **p, **q;
  static void *l[] = { &&lab1, &&lab1, &&lab2, &&lab3, &&lab4 };

  if (!u)
    return l;

  q = *w++;
  goto *q;
lab2:
  p = q;
  q = *w++;
  x = s[2];
  y = s[1];
  z = s[0];
  s -= 1;
  struct S r = foo (x, y, z);
  s[3] = r.n1;
  s[2] = r.n2;
  s[1] = r.n3;
  s[0] = r.n4;
  goto *q;
lab3:
  p = q;
  q = *w++;
  s += 1;
  s[0] = 23;
lab1:
  goto *q;
lab4:
  return 0;
}
#endif

void
testTortureExecute (void)
{
#if 0
  void **u = bar ((void **) 0, (int *) 0);
  void *t[] = { u[2], u[4] };
  int s[] = { 7, 8, 9, 10, 11, 12 };
  if (bar (t, &s[1]) != (void **) 0
      || s[0] != 4 || s[1] != 3 || s[2] != 2 || s[3] != 1
      || s[4] != 11 || s[5] != 12)
    ASSERT (0);
  return;
#endif
}
