/*
   20040707-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct s { char c1, c2; };
void foo (struct s s)
{
  static struct s s1;
  s1 = s;
}

void
testTortureExecute (void)
{
  static struct s s2;
  foo (s2);
  return;
}

