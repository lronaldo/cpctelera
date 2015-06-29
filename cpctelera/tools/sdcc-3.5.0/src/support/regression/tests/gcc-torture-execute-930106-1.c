/*
   930106-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#pragma disable_warning 85
#endif

#define DUMMY_SIZE 9

double g()
{
  return 1.0;
}

f()
{
  char dummy[DUMMY_SIZE];
  double f1, f2, f3;
  f1 = g();
  f2 = g();
  f3 = g();
  return f1 + f2 + f3;
}

void
testTortureExecute (void)
{
  if (f() != 3.0)
    ASSERT(0);
  return;
}

