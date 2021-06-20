/*
   20071205-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR middle-end/34337 */

int
foo (int x)
{
  return ((x << 8) & 65535) | 255;
}

void
testTortureExecute (void)
{
  if (foo (0x32) != 0x32ff || foo (0x174) != 0x74ff)
    ASSERT (0);
  return;
}
