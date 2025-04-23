/*
   20000227-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

static const unsigned char f[] = "\0\377";
static const unsigned char g[] = "\0ÿ";

void
testTortureExecute (void)
{
  if (sizeof f != 3 || sizeof g != 3)
    ASSERT (0);
  if (f[0] != g[0])
    ASSERT (0);
  if (f[1] != g[1])
    ASSERT (0);
  if (f[2] != g[2])
    ASSERT (0);
  return;
}

