/*
   960326-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct s
{
  int a;
  int b;
  short c;
  int d[3];
};

struct s s = { .b = 3, .d = {2,0,0} };

void
testTortureExecute (void)
{
  if (s.b != 3)
    ASSERT (0);
  return;
}

