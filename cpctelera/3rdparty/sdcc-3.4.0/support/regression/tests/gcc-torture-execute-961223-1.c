/*
   961223-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
struct s {
  double d;
};

inline struct s
sub (struct s s)
{
  s.d += 1.0;
  return s;
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct s t = { 2.0 };
  t = sub (t);
  if (t.d != 3.0)
    ASSERT (0);
  return;
#endif
}

