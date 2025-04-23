/*
pr83383.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* PR tree-optimization/83383 */

#if !defined(__SDCC_pdk14) // Lack of memory
unsigned long long int a = 16ULL;
unsigned char b = 195;
unsigned long long int c = ~0ULL;
unsigned char d = 1;
unsigned long long int e[2] = { 3625445792498952486ULL, 0 };
unsigned long long int f[2] = { 0, 8985037393681294663ULL };
unsigned long long int g = 5052410635626804928ULL;

void
foo ()
{
  a = ((signed char) a) < b;
  c = (d ? e[0] : 0) - (f[1] * a ? 1 : g);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory
  foo ();
  if (a != 1 || c != 3625445792498952485ULL)
    ASSERT (0);
  return;
#endif
}
