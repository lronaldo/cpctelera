/*
   20011113-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct assignment!
#if 0
#include <string.h>

typedef struct t
{
  unsigned a : 16;
  unsigned b : 8;
  unsigned c : 8;
  long d[4];
} *T;

typedef struct {
  long r[3];
} U;

T bar (U, unsigned int);

T foo (T x)
{
  U d, u;

  memcpy (&u, &x->d[1], sizeof u);
  d = u;
  return bar (d, x->b);
}

T baz (T x)
{
  U d, u;

  d.r[0] = 0x123456789;
  d.r[1] = 0xfedcba987;
  d.r[2] = 0xabcdef123;
  memcpy (&u, &x->d[1], sizeof u);
  d = u;
  return bar (d, x->b);
}

T bar (U d, unsigned int m)
{
  if (d.r[0] != 21 || d.r[1] != 22 || d.r[2] != 23)
    ASSERT (0);
  return 0;
}

struct t t = { 26, 0, 0, { 0, 21, 22, 23 }};
#endif

void
testTortureExecute (void)
{
#if 0
  baz (&t);
  foo (&t);
  return;
#endif
}

