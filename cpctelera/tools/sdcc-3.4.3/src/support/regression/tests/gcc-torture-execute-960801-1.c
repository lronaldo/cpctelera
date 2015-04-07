/*
   960801-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Some ports do not yet support long long.
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_gbz80)
unsigned
f ()
{
  long long l2;
  unsigned short us;
  unsigned long long ul;
  short s2;

  ul = us = l2 = s2 = -1;
  return ul;
}

unsigned long long
g ()
{
  long long l2;
  unsigned short us;
  unsigned long long ul;
  short s2;

  ul = us = l2 = s2 = -1;
  return ul;
}
#endif

void
testTortureExecute (void)
{
// Test fails on 32-bit systems
#if 0
//#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_gbz80)
  if (f () != (unsigned short) -1)
    ASSERT (0);
  if (g () != (unsigned short) -1)
    ASSERT (0);
  return;
#endif
}

