/*
   pr60072.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/60072 */

int c = 1;

static int *foo (int *p)
{
  return p;
}

int
m ()
{
  *foo (&c) = 2;
  return c - 2;
}

void
testTortureExecute (void)
{
  m();
}

