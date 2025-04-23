/*
   930106-1.c from the execute part of the gcc torture suite.
   What is this file supposed to test?
   Why is there a dummy array?
   Is f() supposed to return an int?
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

//#define DUMMY_SIZE 9

double g(void)
{
  return 1.0;
}

#if !defined( __SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
int f(void)
{
//  char dummy[DUMMY_SIZE];
  double f1, f2, f3;
  f1 = g();
  f2 = g();
  f3 = g();
  return f1 + f2 + f3;
}
#endif

void
testTortureExecute (void)
{
#if !defined( __SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  ASSERT(f() == 3.0);
  return;
#endif
}

