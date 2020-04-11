/*
   pr60003.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/60017 */

struct S0
{
  short m0;
  short m1;
};

struct S1
{
  unsigned m0:1;
  char m1[2][2];
  struct S0 m2[2];
};

struct S1 x = { 1, {{2, 3}, {4, 5}}, {{6, 7}, {8, 9}} };

#if 0 // Enable when SDCC can return struct
struct S1 func (void)
{
  return x;
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct S1 ret = func ();

  if (ret.m2[1].m1 != 9)
    ASSERT (0);
#endif
}
