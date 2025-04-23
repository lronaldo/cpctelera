/*
   931018-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c89
#endif

unsigned int a[/*0x1000*/20];
extern const unsigned long v;

void f (unsigned long a);

void
testTortureExecute (void)
{
  f (v);
  f (v);
  return;
}

void f (unsigned long a)
{
  if (a != 0xdeadbeefL)
    ASSERT (0);
}

const unsigned long v = 0xdeadbeefL;

