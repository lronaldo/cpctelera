/*
   200107231.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
f ()
{
  int biv,giv;
  for (biv = 0, giv = 0; giv != 8; biv++)
      giv = biv*8;
  return giv;
}

void
testTortureExecute (void)
{
  if (f () != 8)
    ASSERT (0);
  return;
}

