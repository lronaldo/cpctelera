/*
   950714-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

int array[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void
testTortureExecute (void)
{
  int i, j;
  int *p;

  for (i = 0; i < 10; i++)
    for (p = &array[0]; p != &array[9]; p++)
      if (*p == i)
	goto label;

 label:
  if (i != 1)
    ASSERT (0);
  return;
}

