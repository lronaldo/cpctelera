/*
   pr69320-4.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdlib.h>

int a;
char b, d;
short c;
short fn1(int p1, int p2) { return p2 >= 2 ? p1 : p1 > p2; }

void
testTortureExecute (void) {
  int *e = &a, *f = &a;
  b = 1;
  for (; b <= 9; b++) {
    c = *e != 5 || d;
    *f = fn1(c || b, a);
  }

  if ((long long) a != 1)
    ASSERT(0);

  return;
}
