/*
   bug-3256.c
   A bug in the stm8 code generation for tail calls from __z88dk_callee with stack parameters.
 */

#include <testfwk.h>

char
g (void)
{
  return 23;
}

char
f (unsigned short a, unsigned short b) __z88dk_callee
{
  if (a == b)
    return g ();
  return 42;
}

void
testBug (void)
{
  ASSERT (f (23, 23) == 23);
  ASSERT (f (23, 42) == 42);
}

