/*
   20021118-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0 // TODO: Enable when struct is supported!
struct s { int f[4]; };

int foo (struct s s, int x1, int x2, int x3, int x4, int x5, int x6, int x7)
{
  return s.f[3] + x7;
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct s s = { 1, 2, 3, 4 };

  if (foo (s, 100, 200, 300, 400, 500, 600, 700) != 704)
    ASSERT (0);
  return;
#endif
}
