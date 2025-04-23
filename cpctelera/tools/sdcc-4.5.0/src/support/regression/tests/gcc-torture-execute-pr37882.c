/*
   pr37882.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR middle-end/37882 */

struct S
{
  unsigned char b : 3;
} s;

void
testTortureExecute (void)
{
  s.b = 4;
  if (s.b > 0 && s.b < 4)
    ASSERT (0);
  return;
}

