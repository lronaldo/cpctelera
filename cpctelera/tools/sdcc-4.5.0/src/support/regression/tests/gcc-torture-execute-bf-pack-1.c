/*
bf-pack-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#if 0 // Enable when SDCC supports bit-fields wider than 16 bits
struct foo
{
  unsigned half:16;
  unsigned long whole:32;
};

f (struct foo *q)
{
  if (q->half != 0x1234)
    ASSERT (0);
  if (q->whole != 0x56789abcL)
    ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
#if 0
  struct foo bar;

  bar.half = 0x1234;
  bar.whole = 0x56789abcL;
  f (&bar);
  return;
#endif
}
