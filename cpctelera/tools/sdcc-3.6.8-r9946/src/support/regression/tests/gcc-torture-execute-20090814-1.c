/*
   20090814-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
bar (int *a)
{
  return *a;
}
int i;
int
foo (int (*a)[2])
{
  return bar (&(*a)[i]);
}

int a[2];
void
testTortureExecute (void)
{
  a[0] = -1;
  a[1] = 42;
  i = 1;
  if (foo (&a) != 42)
    ASSERT (0);
  return;
}

