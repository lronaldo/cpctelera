/*
pr85529-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* PR tree-optimization/85529 */

struct S { int a; };

int b, c = 1, d, e, f;
static int g;
volatile struct S s;

signed char
foo (signed char i, int j)
{
  return i < 0 ? i : i << j;
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && (__GNUC__ < 8))
  signed char k = -83;
  if (!d)
    goto L;
  k = e || f;
L:
  for (; b < 1; b++)
    s.a != (k < foo (k, 2) && (c = k = g));
  if (c != 1)
    ASSERT (0);
  return;
#endif
}
