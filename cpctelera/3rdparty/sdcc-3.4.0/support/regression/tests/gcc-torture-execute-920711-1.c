/*
   920711-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

f(long a)
{
  return (--a > 0);
}

void
testTortureExecute (void)
{
#if !(defined (__GNUC__) && (__GNUC__ < 5))
  if(f (0x80000000L) == 0)
    ASSERT(0);
  return;
#endif
}

