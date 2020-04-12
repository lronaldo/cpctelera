/*
   loop-10.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Reduced from PR optimization/5076, PR optimization/2847 */

static int count = 0;

static void
inc (void)
{
  count++;
}

void
testTortureExecute (void)
{
  int iNbr = 1;
  int test = 0;
  while (test == 0)
    {
      inc ();
      if (iNbr == 0)
        break;
      else
        {
          inc ();
          iNbr--;
        }
      test = 1;
    }
  if (count != 2)
    ASSERT (0);
  return;
}
