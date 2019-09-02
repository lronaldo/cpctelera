/*
   pr64682.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR rtl-optimization/64682 */

int a, b = 1;

void
foo (int x)
{
  if (x != 5)
    ASSERT (0);
}

void
testTortureExecute (void)
{
  int i;
  for (i = 0; i < 56; i++)
    for (; a; a--)
      ;
#if 0 //Enabel when SDCC supports intermingling of statements and declarations
  int *c = &b;
  if (*c)
    *c = 1 % (unsigned int) *c | 5;

  foo (b);
#endif
  return;
}
