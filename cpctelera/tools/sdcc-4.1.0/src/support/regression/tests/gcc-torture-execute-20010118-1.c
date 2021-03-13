/*
   20010118-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#pragma disable_warning 93
#endif

typedef struct {
  int a, b, c, d, e, f;
} A;

void foo (A *v, int w, int x, int *y, int *z)
{
}

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
void
bar (A *v, int x, int y, int w, int h)
{
  if (v->a != x || v->b != y) {
    int oldw = w;
    int oldh = h;
    int e = v->e;
    int f = v->f;
    int dx, dy;
    foo(v, 0, 0, &w, &h);
    dx = (oldw - w) * (double) e/2.0;
    dy = (oldh - h) * (double) f/2.0;
    x += dx;
    y += dy;
    v->a = x;
    v->b = y;
    v->c = w;
    v->d = h;
  }
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  A w = { 100, 110, 20, 30, -1, -1 };
  bar (&w,400,420,50,70);
  if (w.d != 70)
    ASSERT(0);
  return;
#endif
}

