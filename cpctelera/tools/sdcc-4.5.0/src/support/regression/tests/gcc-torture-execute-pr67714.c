/*
   pr67714.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 84
#endif

unsigned int b;
int c;

signed char
fn1 ()
{
  signed char d;
  for (int i = 0; i < 1; i++)
    d = -15;
  return d;
}

void
testTortureExecute (void)
{
  for (c = 0; c < 1; c++)
    b = 0;
#if 0 // Enable when SDCC intermingles
  char e = fn1 ();
  signed char f = e ^ b;
  volatile int g = (int) f;

  ASSERT (g == -15);
#endif
  return;
}
