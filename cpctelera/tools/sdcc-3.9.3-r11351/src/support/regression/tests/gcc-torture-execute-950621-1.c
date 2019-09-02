/*
   950621-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct s
{
  int a;
  int b;
  struct s *dummy;
};

f (struct s *sp)
{
  return sp && sp->a == -1 && sp->b == -1;
}

void
testTortureExecute (void)
{
  struct s x;
  x.a = x.b = -1;
  if (f (&x) == 0)
    ASSERT (0);
  return;
}

