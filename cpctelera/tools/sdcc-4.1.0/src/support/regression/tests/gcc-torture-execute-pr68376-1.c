/*
   pr68376-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

/* PR rtl-optimization/68376 */

int a, b, c = 1;
signed char d;

void
testTortureExecute (void)
{
  for (; a < 1; a++)
    for (; b < 1; b++)
      {
	signed char e = ~d;
	if (d < 1)
	  e = d;
	d = e;
	if (!c)
	  ASSERT (0);
      }

  if (d != 0)
    ASSERT (0);

  return;
}
