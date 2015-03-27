/*
   20060929-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR c/29154 */

void
foo (int **p, int *q)
{
  *(*p++)++ = *q++;
}

void
bar (int **p, int *q)
{
  **p = *q++;
  *(*p++)++;
}

void
baz (int **p, int *q)
{
  **p = *q++;
  (*p++)++;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && (__GNUC__ < 5))
  int i = 42, j = 0;
  int *p = &i;
  foo (&p, &j);
  if (p - 1 != &i || j != 0 || i != 0)
    ASSERT (0);
  i = 43;
  p = &i;
  bar (&p, &j);
  if (p - 1 != &i || j != 0 || i != 0)
    ASSERT (0);
  i = 44;
  p = &i;
  baz (&p, &j);
  if (p - 1 != &i || j != 0 || i != 0)
    ASSERT (0);
  return;
#endif
}
