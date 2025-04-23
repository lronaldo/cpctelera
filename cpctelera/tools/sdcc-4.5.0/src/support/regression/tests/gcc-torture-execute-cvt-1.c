/*
cvt-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 93
#endif

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
static inline long
g1 (double x)
{
  return (double) (long) x;
}

long
g2 (double f)
{
  return f;
}

double
f (long i)
{
  if (g1 (i) != g2 (i))
    ASSERT (0);
  return g2 (i);
}
#endif

void
testTortureExecute (void)
{
#if 0 // TODO: Enable when SDCC supports double!
  if (f (123456789L) != 123456789L)
    ASSERT (0);
  if (f (123456789L) != g2 (123456789L))
    ASSERT (0);
  return;
#endif
}
