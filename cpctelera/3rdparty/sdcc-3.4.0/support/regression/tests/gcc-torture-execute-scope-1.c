/*
   scope-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int v = 3;

void f (void)
{
  int v = 4;
  {
    extern int v;
    if (v != 3)
      ASSERT (0);
  }
}

void
testTortureExecute (void)
{
  f ();
  return;
}

