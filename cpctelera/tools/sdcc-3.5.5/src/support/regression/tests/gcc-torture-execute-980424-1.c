/*
   980424-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#ifndef __SDCC_mcs51
int i, a[99];

void f (int one)
{
  if (one != 1)
    ASSERT (0);
}

void
g ()
{
  f (a[i & 0x3f]);
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_mcs51
  a[0] = 1;
  i = 0x40;
  g ();
  return;
#endif
}

