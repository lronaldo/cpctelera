/*
   20060102-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

int f(int x)
{
  return (x >> (sizeof (x) * CHAR_BIT - 1)) ? -1 : 1;
}

volatile int one = 1;
void testTortureExecute (void)
{
  /* Test that the function above returns different values for
     different signs.  */
  if (f(one) == f(-one))
    ASSERT (0);
  return;
}

