/*
   20120111-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdlib.h>
#include <stdint.h>

uint32_t f0a (uint64_t arg2);

uint32_t
f0a (uint64_t arg)
{
  return ~((unsigned) (arg > -3));
}

void
testTortureExecute (void)
{
  uint32_t r1;
  r1 = f0a (12094370573988097329ULL);
  if (r1 != ~0U)
    ASSERT(0);
}

