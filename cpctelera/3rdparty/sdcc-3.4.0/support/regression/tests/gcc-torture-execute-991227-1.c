/*
   991227-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

char* doit(int flag)
{
  return 1 + (flag ? "\0wrong\n" : "\0right\n");
}

void
testTortureExecute (void)
{
  char *result = doit(0);
  if (*result == 'r' && result[1] == 'i')
    return;
  ASSERT (0);
}

