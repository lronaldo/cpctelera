/*
   pr39120.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0
SDCC urrently cannot return aggregates TODO: Enable later.
struct X { int *p; } x;

struct X
foo(int *p) { struct X x; x.p = p; return x; }

void
bar() { *x.p = 1; }
#endif

void
testTortureExecute (void)
{
#if 0
  int i = 0;
  x = foo(&i);
  bar();
  if (i != 1)
    ASSERT (0);
  return;
#endif
}
