/*
   20001228-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int foo1(void)
{
  union {
    char a[sizeof (unsigned)];
    unsigned b;
  } u;
  
  u.b = 0x01;
  return u.a[0];
}

int foo2(void)
{
  volatile union {
    char a[sizeof (unsigned)];
    unsigned b;
  } u;
  
  u.b = 0x01;
  return u.a[0];
}

void
testTortureExecute (void)
{
  if (foo1() != foo2())
    ASSERT (0);
  return;
}

