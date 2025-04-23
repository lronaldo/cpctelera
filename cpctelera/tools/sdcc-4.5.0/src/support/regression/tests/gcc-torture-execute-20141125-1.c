/*
   20141125-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int f(long long a);
int f(long long a)
{
  if (a & 0x3ffffffffffffffull)
    return 1;
  return 1024;
}

void
testTortureExecute (void)
{
  if(f(0x48375d8000000000ull) != 1)
    ASSERT(0);
  if (f(0xfc00000000000000ull) != 1024)
    ASSERT(0);
}

