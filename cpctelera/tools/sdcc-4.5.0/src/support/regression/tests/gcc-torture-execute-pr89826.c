/*
   pr89826.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

typedef unsigned int u32;
typedef unsigned long long u64;
u64 a;
u32 b;

u64
foo (u32 d)
{
  a -= d ? 0 : ~a;
  return a + b;
}

void
testTortureExecute (void)
{
  u64 x = foo (2);
  if (x != 0)
    ASSERT (0);
  return;
}

