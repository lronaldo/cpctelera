/*
   bug-2263.c
*/

#include <testfwk.h>

unsigned long g()
{
  return 0xFFFFFFFF;
}

void f(unsigned short m)
{
  unsigned long e = g() + m;
  while (e)
    ASSERT (0);
}

void testBug (void)
{
#ifdef __SDCC
  f(1);
#endif
}
