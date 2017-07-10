/*
   20040707-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: enable when struct can be passed!
struct s { char c1, c2; };
void foo (struct s s)
{
  static struct s s1;
  s1 = s;
}
#endif

void
testTortureExecute (void)
{
#if 0
  static struct s s2;
  foo (s2);
  return;
#endif
}
