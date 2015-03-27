/*
   20031216-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR optimization/13313 */
/* Origin: Mike Lerwill <mike@ml-solutions.co.uk> */

void DisplayNumber (unsigned long v)
{
  if (v != 0x9aL)
    ASSERT (0);
}

unsigned long ReadNumber (void)
{
  return 0x009a0000L;
}

void
testTortureExecute (void)
{
  unsigned long tmp;
  tmp = (ReadNumber() & 0x00ff0000L) >> 16;
  DisplayNumber (tmp);
  return;
}

