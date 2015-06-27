/*
   960116-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static inline int
p (int *p)
{
  return !((long) p & 1);
}

int
f (int *q)
{
  if (p (q) && *q)
    return 1;
  return 0;
}

void
testTortureExecute (void)
{
  ASSERT (f ((int __code *) 0xffffffff) == 0);
  return;
}
