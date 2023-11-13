/*
   pr69320-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdlib.h>
int a, b, d, f;
char c;
static int *e = &d;
void
testTortureExecute (void) {
  int g = -1L;
  *e = g;
  c = 4;
  for (; c >= 14; c++)
    *e = 1;
  f = a == 0;
  *e ^= f;
#if 0 // Enable when SDCC intermingles
  int h = ~d;
  if (d)
    b = h;
  if (h)
    return;
  ASSERT(0);
#endif
}

