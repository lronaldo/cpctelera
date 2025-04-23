/*
   divconst-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef struct
{
  unsigned a, b, c, d;
} t1;

void
f (t1 *ps)
{
    ps->a = 10000;
    ps->b = ps->a / 3;
    ps->c = 10000;
    ps->d = ps->c / 3;
}

void
testTortureExecute (void)
{
  t1 s;
  f (&s);
  ASSERT(!(s.a != 10000 || s.b != 3333 || s.c != 10000 || s.d != 3333));
  return;
}
