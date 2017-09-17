/*
   960218-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int globl;

int g (int x)
{
  globl = x;
  return 0;
}

void f (int x)
{
  int a = ~x;
  while (a)
    a = g (a);
}

void
testTortureExecute (void)
{
  f (3);
  ASSERT (globl == -4);
  return;
}
