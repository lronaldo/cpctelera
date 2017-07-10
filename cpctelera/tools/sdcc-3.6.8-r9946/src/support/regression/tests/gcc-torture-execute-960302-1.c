/*
   960302-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

long a = 1;

foo ()
{
  switch (a % 2 % 2 % 2 % 2 % 2 % 2 % 2 % 2)
    {
    case 0:
      return 0;
    case 1:
      return 1;
    default:
      return -1;
    }
}

void
testTortureExecute (void)
{
  if (foo () != 1)
    ASSERT (0);
  return;
}

