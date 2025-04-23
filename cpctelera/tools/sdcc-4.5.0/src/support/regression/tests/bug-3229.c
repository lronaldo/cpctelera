/*
   bug-3229.c
   A bug in code generation for __z88dk_callee functions returning long long
 */
 
#include <testfwk.h>

long long f0(void) __z88dk_callee
{
  return 0xa5;
}

long long f1(int i) __z88dk_callee
{
  return 0xa5 + i;
}

long long f4(long long l) __z88dk_callee
{
  return 0xa5 + l;
}

void
testBug (void)
{
  volatile long long l = 0xaa55;
  ASSERT (f0()  == 0xa5);
  ASSERT (f1(1) == 0xa6);
  ASSERT (f4(2) == 0xa7);
  ASSERT (l == 0xaa55);
}

