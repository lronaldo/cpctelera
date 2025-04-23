/*
   pr81556.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pdk14) // Lack of memory
/* PR tree-optimization/81556 */
unsigned long long int b = 0xb82ff73c5c020599ULL;
unsigned long long int c = 0xd4e8188733a29d8eULL;
unsigned long long int d = 2, f = 1, g = 0, h = 0;
unsigned long long int e = 0xf27771784749f32bULL;

void
foo (void)
{
  _Bool a = d > 1;
  g = f % ((d > 1) << 9);
  h = a & (e & (a & b & c));
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory
  foo ();
  if (g != 1 || h != 0)
    ASSERT (0);
  return;
#endif
}
