/*
   981130-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
struct s { int a; int b;};
struct s s1;
struct s s2 = { 1, 2, };

void
check (int a, int b)
{
  if (a == b)
    return;
  else
    ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
#if 0
  int * p;
  int x;
  
  s1.a = 9;
  p    = & s1.a;
  s1   = s2;
  x    = * p;
  
  check (x, 1);
#endif
}

