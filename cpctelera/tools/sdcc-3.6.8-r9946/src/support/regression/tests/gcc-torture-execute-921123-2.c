/*
   921123-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
typedef struct
{
  unsigned short b0, b1, b2, b3;
} four_quarters;

four_quarters x;
int a, b;

void f (four_quarters j)
{
  b = j.b2;
  a = j.b3;
}
#endif

void
testTortureExecute (void)
{
#if 0
  four_quarters x;
  x.b0 = x.b1 = x.b2 = 0;
  x.b3 = 38;
  f(x);
  if (a != 38)
    ASSERT(0);
  return;
#endif
}

