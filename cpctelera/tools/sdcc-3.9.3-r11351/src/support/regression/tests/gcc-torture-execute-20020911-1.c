/*
   20020911-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

unsigned short c = 0x8000;
void
testTortureExecute (void)
{
  if ((c-0x8000) < 0 || (c-0x8000) > 0x7fff)
    ASSERT(0);
  return;
}

