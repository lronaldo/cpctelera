/*
   pr79388.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/79388 */
/* { dg-additional-options "-fno-tree-coalesce-vars" } */

unsigned int a, c;

unsigned int
foo (unsigned int p)
{
  p |= 1;
  p &= 0xfffe;
  p %= 0xffff;
  c = p;
  return a + p;
}

void
testTortureExecute (void)
{
  int x = foo (6);
  if (x != 6)
    ASSERT (0);
  return;
}
