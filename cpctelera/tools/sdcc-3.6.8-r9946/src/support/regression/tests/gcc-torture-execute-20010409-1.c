/*
   20010409-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

#include <string.h>
#include <setjmp.h>

jmp_buf try;

typedef struct A {
  int a, b;
} A;

typedef struct B {
  struct A **a;
  int b;
} B;

A *a;
int b = 1, c;
B d[1];

void foo (A *x, const char *y, int z)
{
  c = y[4] + z * 25;
}

A *bar (const char *v, int w, int x, const char *y, int z)
{
  if (w)
    ASSERT (0);
  longjmp(try, 1);
}

void test (const char *x, int *y)
{
  foo (d->a[d->b], "test", 200);
  d->a[d->b] = bar (x, b ? 0 : 65536, strlen (x), "test", 201);
  d->a[d->b]->a++;
  if (y)
    d->a[d->b]->b = *y;
}

void
testTortureExecute (void)
{
  if (setjmp(try) == 0)
  {
    d->b = 0;
    d->a = &a;
    test ("", 0);
  }
}

