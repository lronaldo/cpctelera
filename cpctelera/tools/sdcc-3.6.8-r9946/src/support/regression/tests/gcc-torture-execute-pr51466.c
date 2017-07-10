/*
   pr51466.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/51466 */

int
foo (int i)
{
  volatile int v[4];
  int *p;
  v[i] = 6;
  p = (int *) &v[i];
  return *p;
}

int
bar (int i)
{
  volatile int v[4];
  int *p;
  v[i] = 6;
  p = (int *) &v[i];
  *p = 8;
  return v[i];
}

int
baz (int i)
{
  volatile int v[4];
  int *p;
  v[i] = 6;
  p = (int *) &v[0];
  *p = 8;
  return v[i];
}

void
testTortureExecute (void)
{
  if (foo (3) != 6 || bar (2) != 8 || baz (0) != 8 || baz (1) != 6)
    ASSERT (0);
  return;
}

