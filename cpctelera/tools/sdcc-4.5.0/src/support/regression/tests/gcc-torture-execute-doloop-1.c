/*
   doloop-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <limits.h>

volatile unsigned int i;

void
testTortureExecute (void)
{
  unsigned char z = 0;

  do ++i;
  while (--z > 0);
  if (i != UCHAR_MAX + 1U)
    ASSERT (0);
  return;
}
