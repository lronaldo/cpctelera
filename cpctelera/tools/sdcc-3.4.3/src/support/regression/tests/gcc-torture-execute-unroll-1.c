/*
   unroll-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static
inline int
f (int x)
{
  return (x + 1);
}
 
void
testTortureExecute (void)
{
  int a = 0 ;
 
  while ( (f(f(f(f(f(f(f(f(f(f(1))))))))))) + a < 12 )
    {
      a++;
      return;
    }
  if (a != 1)
    ASSERT (0);
}

