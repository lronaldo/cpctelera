/*
   pr61306-3.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

short a = -1;
int b;
char c;

void
testTortureExecute (void)
{
  c = a;
  b = a | c;
  if (b != -1)
    ASSERT (0);
}
