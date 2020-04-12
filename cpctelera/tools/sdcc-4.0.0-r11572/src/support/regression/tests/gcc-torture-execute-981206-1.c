/*
   981206-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Verify unaligned address aliasing on Alpha EV[45].  */

static unsigned short x, y;

void foo()
{
  x = 0x345;
  y = 0x567;
}

void
testTortureExecute (void)
{
  foo ();
  if (x != 0x345 || y != 0x567)
    ASSERT (0);
  return;
}

