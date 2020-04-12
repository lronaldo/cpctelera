/*
   921124-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#pragma disable_warning 85
#endif

#if !defined(__SDCC_pic14) // Pseudo-stack size limit
int f(int x, double d1, double d2, double d3)
{
   return x;
}

void g(char *b, char *s, double x, double y, int i, int j)
{
#if !defined(__SDCC_pdk14) // Lack of memory
  if (x != 1.0 || y != 2.0 || i != 3 || j != 4)
    ASSERT(0);
#endif
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pic14) // Pseudo-stack size limit
  g("","", 1.0, 2.0, f(3, 0.0, 0.0, 0.0), f(4, 0.0, 0.0, 0.0));
  return;
#endif
}

