/*
   960301-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct foo {
  unsigned : 12;
  unsigned field : 4;
} foo;
unsigned oldfoo;

int
bar (unsigned k)
{
  oldfoo = foo.field;
  foo.field = k;
  if (k)
    return 1;
  return 2;
}

void
testTortureExecute (void)
{
  if (bar (1U) != 1)
    ASSERT (0);
  return;
}

