/*
   930818-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

static double one = 1.0;

int f()
{
  int colinear;
  colinear = (one == 0.0);
  if (colinear)
    ASSERT (0);
  return colinear;
}
void
testTortureExecute (void)
{
  if (f()) ASSERT (0);
  return;
}

