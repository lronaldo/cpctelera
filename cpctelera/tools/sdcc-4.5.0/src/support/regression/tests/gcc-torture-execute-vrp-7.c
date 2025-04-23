/*
vrp-7.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

struct T
{
  int b : 1;
} t;

void foo (int f)
{
  t.b = (f & 0x10) ? 1 : 0;
}

void
testTortureExecute (void)
{
  foo (0x10);
  if (!t.b)
    ASSERT (0);
  return;
}
