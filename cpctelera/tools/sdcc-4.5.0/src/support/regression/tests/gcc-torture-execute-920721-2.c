/*
   920721-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// Todo. Enable when sdcc supports VLA!
#if 0
void f(void){}
#endif

void
testTortureExecute (void)
{
#if 0
  int n=2;
  double x[n];
  f();
  return;
#endif
}

