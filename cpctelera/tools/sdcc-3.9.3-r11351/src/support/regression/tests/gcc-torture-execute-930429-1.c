/*
   930429-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

const char *
f (const char *p)
{
  short x = *p++ << 16;
  return p;
}

void
testTortureExecute (void)
{
  const char *p = "";
  if (f (p) != p + 1)
    ASSERT(0);
  return;
}

