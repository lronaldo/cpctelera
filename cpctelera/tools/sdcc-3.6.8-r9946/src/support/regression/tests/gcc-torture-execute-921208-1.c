/*
   921208-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

#if !defined(__SDCC_hc08) && !defined(__SDCC_s08)
double
f(double x)
{
  return x*x;
}

double
Int(double (*f)(double), double a)
{
  return (*f)(a);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08)
  if (Int(&f,2.0) != 4.0)
    ASSERT(0);
  return;
#endif
}

