/*
   20030626-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
char buf[40];
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  int i = 0;
  int l = sprintf (buf, "%s", i++ ? "string" : "other string");
  if (l != sizeof ("other string") - 1 || i != 1)
    ASSERT (0);
  return;
#endif
}

