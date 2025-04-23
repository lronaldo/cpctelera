/*
cmpsi-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

void dummy (void) {}

int
f1 (unsigned int x, unsigned int y)
{
  if (x == 0)
    dummy ();
  x -= y;
  /* 0xfffffff2 < 0x80000000? */
  ASSERT(!(x < ~(~(unsigned int) 0 >> 1)));
  return x;
}

int
f2 (unsigned long int x, unsigned long int y)
{
  if (x == 0)
    dummy ();
  x -= y;
  /* 0xfffffff2 < 0x80000000? */
  ASSERT(!(x < ~(~(unsigned long int) 0 >> 1)));
  return x;
}

void
testTortureExecute (void)
{
  /*      0x7ffffff3			0x80000001 */
  f1 ((~(unsigned int) 0 >> 1) - 12, ~(~(unsigned int) 0 >> 1) + 1);
  f2 ((~(unsigned long int) 0 >> 1) - 12, ~(~(unsigned long int) 0 >> 1) + 1);
  return;
}
