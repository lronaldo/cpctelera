/*
   pr38212.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
foo (int *restrict p, int i)
{
  int *restrict q;
  int *restrict r;
  int v, w;
  q = p + 1;
  r = q - i;
  v = *r;
  *p = 1;
  w = *r;
  return v + w;
}

void
testTortureExecute (void)
{
#if 0
  int i = 0;
  if (foo (&i, 1) != 1)
    ASSERT (0);
  return;
#endif
}

