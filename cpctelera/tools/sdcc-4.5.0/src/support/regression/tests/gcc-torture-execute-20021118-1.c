/*
   20021118-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#pragma disable_warning 85

#ifdef __SDCC
#pragma std_c99
#endif

struct s { int f[4]; };

int foo (struct s s, int x1, int x2, int x3, int x4, int x5, int x6, int x7)
{
  return s.f[3] + x7;
}

void
testTortureExecute (void)
{
#if 0 // Todo: enable when brace omission in struct initialization is supported!
  struct s s = { 1, 2, 3, 4 };

  if (foo (s, 100, 200, 300, 400, 500, 600, 700) != 704)
    ASSERT (0);
  return;
#endif
}

