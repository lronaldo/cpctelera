/*
   pr49419.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

/* PR tree-optimization/49419 */

struct S { int w, x, y; } *t;

int
foo (int n, int f, int *s, int m)
{
  int x, i, a;
  if (n == -1)
    return 0;
  for (x = n, i = 0; t[x].w == f && i < m; i++)
    x = t[x].x;
  if (i == m)
    ASSERT (0);
  a = i + 1;
  for (x = n; i > 0; i--)
    {
      s[i] = t[x].y;
      x = t[x].x;
    }
  s[0] = x;
  return a;
}

void
testTortureExecute (void)
{
  int s[3], i;
  struct S buf[3] = { { 1, 1, 2 }, { 0, 0, 0 }, { 0, 0, 0 } };
  t = buf;
  if (foo (0, 1, s, 3) != 2)
    ASSERT (0);
  if (s[0] != 1 || s[1] != 2)
    ASSERT (0);
  return;
}

