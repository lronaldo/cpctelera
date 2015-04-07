/*
   920428-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

x (const char*s)
{
  char a[1];
  const char *ss = s;
  a[*s++] |= 1;
  return (int)ss + 1 == (int)s;
}

void
testTortureExecute (void)
{
  if (x("") != 1)
    ASSERT(0);
  return;
}

