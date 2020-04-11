/*
   930603-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
float fx (float x)
{
  return 1.0 + 3.0 / (2.302585093 * x);
}

float inita ();
float initc ();
void f ();
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  float a, b, c;
  a = inita ();
  c = initc ();
  f ();
  b = fx (c) + a;
  f ();
  if (a != 3.0 || b < 4.3257 || b > 4.3258 || c != 4.0)
    ASSERT (0);
  return;
#endif
}

float inita () { return 3.0; }
float initc () { return 4.0; }
void f () {}

