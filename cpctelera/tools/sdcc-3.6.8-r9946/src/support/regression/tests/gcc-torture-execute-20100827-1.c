/*
   20100827-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int
foo (char *p)
{
  int h = 0;
  do
    {
      if (*p == '\0')
	break;
      ++h;
      if (p == 0)
	ASSERT (0);
      ++p;
    }
  while (1);
  return h;
}

void
testTortureExecute (void)
{
  if (foo("a") != 1)
    ASSERT (0);
  return;
}

