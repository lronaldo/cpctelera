/*
widechar-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#define C L'\400'

#if C
#define zero (!C)
#else
#define zero C
#endif

void
testTortureExecute (void)
{
  if (zero != 0)
    ASSERT (0);
  return;
}
