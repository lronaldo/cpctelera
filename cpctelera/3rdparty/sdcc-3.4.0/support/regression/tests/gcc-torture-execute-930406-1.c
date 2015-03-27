/*
   930406-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

void f()
{
  int x = 1;
#ifndef __SDCC_mcs51
  char big[2048];
#else
  char big[2];
#endif

  {
  mylabel:
    x++;
    if (x != 3)
      goto mylabel;
  }
  return;
}

void
testTortureExecute (void)
{
  f();
}

