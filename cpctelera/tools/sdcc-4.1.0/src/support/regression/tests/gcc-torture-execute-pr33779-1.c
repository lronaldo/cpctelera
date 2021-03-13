/*
   pr33779-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdint.h>

int foo(int32_t i)
{
  if (((uint32_t)(i + 1)) * 4 == 0)
    return 1;
  return 0;
}

void
testTortureExecute (void)
{
  if (foo(0x3fffffff) == 0)
    ASSERT (0);
  return;
}

