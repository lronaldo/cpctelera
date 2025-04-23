/*
   960311-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>

int count;

void a1() { ++count; }

void
b (unsigned short data)
{
  if (data & 0x8000) a1();
  data <<= 1;

  if (data & 0x8000) a1();
  data <<= 1;

  if (data & 0x8000) a1();
}

void
testTortureExecute (void)
{
  count = 0;
  b (0);
  if (count != 0)
    ASSERT (0);

  count = 0;
  b (0x8000);
  if (count != 1)
    ASSERT (0);

  count = 0;
  b (0x4000);
  if (count != 1)
    ASSERT (0);

  count = 0;
  b (0x2000);
  if (count != 1)
    ASSERT (0);

  count = 0;
  b (0xc000);
  if (count != 2)
    ASSERT (0);

  count = 0;
  b (0xa000);
  if (count != 2)
    ASSERT (0);

  count = 0;
  b (0x6000);
  if (count != 2)
    ASSERT (0);

  count = 0;
  b (0xe000);
  if (count != 3)
    ASSERT (0);

  return;
}

