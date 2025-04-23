/*
   960311-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdio.h>

int count;

void a1() { ++count; }

void
b (unsigned char data)
{
  if (data & 0x80) a1();
  data <<= 1;

  if (data & 0x80) a1();
  data <<= 1;

  if (data & 0x80) a1();
}

void
testTortureExecute (void)
{
  count = 0;
  b (0);
  if (count != 0)
    ASSERT (0);

  count = 0;
  b (0x80);
  if (count != 1)
    ASSERT (0);

  count = 0;
  b (0x40);
  if (count != 1)
    ASSERT (0);

  count = 0;
  b (0x20);
  if (count != 1)
    ASSERT (0);

  count = 0;
  b (0xc0);
  if (count != 2)
    ASSERT (0);

  count = 0;
  b (0xa0);
  if (count != 2)
    ASSERT (0);

  count = 0;
  b (0x60);
  if (count != 2)
    ASSERT (0);

  count = 0;
  b (0xe0);
  if (count != 3)
    ASSERT (0);

  return;
}

