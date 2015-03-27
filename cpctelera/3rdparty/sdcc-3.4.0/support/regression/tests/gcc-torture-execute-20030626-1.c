/*
   20030626-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>

char buf[10];

void
testTortureExecute (void)
{
  int l = sprintf (buf, "foo\0bar");
  if (l != 3)
    ASSERT (0);
  return;
}

