/*
   990604-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int b;
void f ()
{
  int i = 0;
  if (b == 0)
    do {
      b = i;
      i++;
    } while (i < 10);
}

void
testTortureExecute (void)
{
  f ();
  if (b != 9)
    ASSERT (0);
  return;
}

