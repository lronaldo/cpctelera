/*
   pr23604.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int g(int i, int j)
{
  if (i>-1)
    if (i<2)
     {
        if (i != j)
          {
            if (j != 0)
                return 0;
          }
     }
  return 1;
}

void
testTortureExecute (void)
{
  if (!g(1, 0))
   ASSERT (0);
  return;
}

