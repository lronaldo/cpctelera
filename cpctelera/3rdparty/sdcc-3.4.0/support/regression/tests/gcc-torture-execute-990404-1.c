/*
   990404-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 84
#pragma disable_warning 85
#endif


int x[10] = { 0,1,2,3,4,5,6,7,8,9};

void
testTortureExecute (void)
{
  int niterations = 0, i;

  for (;;) {
    int i, mi, max;
    max = 0;
    for (i = 0; i < 10 ; i++) {
      if (x[i] > max) {
        max = x[i];
        mi = i;
      }
    }
    if (max == 0)
      break;
    x[mi] = 0;
    niterations++;
    if (niterations > 10)
      ASSERT (0);
  }

  return;
}
