/*
   pr15262.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct S {
  unsigned b4:4;
  unsigned b6:6;
} s;

void
testTortureExecute (void)
{
  s.b6 = 31;
  s.b4 = s.b6;
  s.b6 = s.b4;
  if (s.b6 != 15)
    ASSERT (0);
  return;
}

