/*
   980604-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

int a = 1;
int b = -1;

int c = 1;
int d = 0;

void
testTortureExecute (void)
{
  double e;
  double f;
  double g;

  f = c;
  g = d;
  e = (a < b) ? f : g;
  if (e)
    ASSERT (0);
  return;
}

