/*
   20060930-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

/* PR middle-end/29272 */

struct S { struct S *s; } s;
struct T { struct T *t; } t;

static inline void
foo (void *s)
{
  struct T *p = s;
  memcpy (&p->t, &t.t, sizeof (t.t));
}

void *
bar (void *p, struct S *q)
{
  q->s = &s;
  foo (p);
  return q->s;
}

void
testTortureExecute (void)
{
  t.t = &t;
  if (bar (&s, &s) != (void *) &t)
    ASSERT (0);
  return;
}
