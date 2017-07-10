/*
   pr49161.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR tree-optimization/49161 */

int c;

void
bar (int x)
{
  if (x != c++)
    ASSERT (0);
}

void
foo (int x)
{
  switch (x)
    {
    case 3: goto l1;
    case 4: goto l2;
    case 6: goto l3;
    default: return;
    }
l1:
  goto l4;
l2:
  goto l4;
l3:
  bar (-1);
l4:
  bar (0);
  if (x != 4)
    bar (1);
  if (x != 3)
    bar (-1);
  bar (2);
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && defined (__GNUC_MINOR__) && (__GNUC__ < 5))
  foo (3);
  if (c != 3)
    ASSERT (0);
  return;
#endif
}

