/*
   961004-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 84
#endif

int k = 0;

void
testTortureExecute (void)
{
  int i;
  int j;

  for (i = 0; i < 2; i++)
    {
      if (k)
        {
          if (j != 2)
            ASSERT (0);
        }
      else
        {
          j = 2;
          k++;
        }
    }
  return;
}
