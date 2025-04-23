/*
   20000715-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned int foo(unsigned int a)
{
  return ((unsigned char)(a + 1)) * 4;
}

void
testTortureExecute (void)
{
  if (foo((unsigned char)~0))
    ASSERT (0);
  return;
}

