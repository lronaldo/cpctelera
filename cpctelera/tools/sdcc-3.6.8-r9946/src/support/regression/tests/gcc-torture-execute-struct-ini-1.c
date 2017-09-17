/*
   struct-ini-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if 0
struct S
{
  char f1;
  int f2[2];
};

struct S object = {'X', 8, 9};
#endif

void
testTortureExecute (void)
{
#if 0
  if (object.f1 != 'X' || object.f2[0] != 8 || object.f2[1] != 9)
    ASSERT (0);
  return;
#endif
}

