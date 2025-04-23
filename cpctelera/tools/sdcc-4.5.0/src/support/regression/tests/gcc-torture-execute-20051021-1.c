/*
   20051021-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Verify that TRUTH_AND_EXPR is not wrongly changed to TRUTH_ANDIF_EXPR.  */

int count = 0;

int foo1(void)
{
  count++;
  return 0;
}

int foo2(void)
{
  count++;
  return 0;
}

void testTortureExecute(void)
{
  if ((foo1() == 1) & (foo2() == 1))
    ASSERT (0);

  if (count != 2)
    ASSERT (0);

  return;
}
