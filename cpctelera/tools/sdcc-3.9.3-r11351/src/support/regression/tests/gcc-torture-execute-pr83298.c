/*
pr83298.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>


int a, b, c = 1;

void
testTortureExecute (void)
{
  for (; b < 1; b++)
    ;
  if (!(c * (a < 1))) 
    ASSERT (0);
  return; 
}
