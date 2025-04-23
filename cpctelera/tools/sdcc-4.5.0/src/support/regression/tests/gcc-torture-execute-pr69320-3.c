/*
   pr69320-3.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdlib.h>

#if !defined(__SDCC_pdk14) // Lack of memory
static int a[40] = {7, 5, 3, 3, 0, 0, 3};
short b;
int c = 5;
#endif

void
testTortureExecute (void) {
#if !defined(__SDCC_pdk14) // Lack of memory
  b = 0;
  for (; b <= 3; b++)
    if (a[b + 6] ^ (0 || c))
      ;
    else
      break;
  if (b != 4)
    ASSERT(0);
  return;
#endif
}

