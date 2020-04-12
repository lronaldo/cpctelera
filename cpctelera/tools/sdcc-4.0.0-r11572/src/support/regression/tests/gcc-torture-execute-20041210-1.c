/*
   20041210-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* The FR-V port used to fail this test because the andcc patterns
   wrongly claimed to set the C and V flags.  */
#include <limits.h>

int x[4] = { INT_MIN / 2, INT_MAX, 2, 4 };

void
testTortureExecute (void)
{
  if (x[0] < x[1])
    if ((x[2] & x[3]) < 0)
      ASSERT (0);
  return;
}
