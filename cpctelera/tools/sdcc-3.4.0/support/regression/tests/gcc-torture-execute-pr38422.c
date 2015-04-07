/*
   pr38411.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#ifndef __SDCC_pic16 // TODO: enable when the pic16 ports supports bitfields of size greater than 8 bits!
#include <limits.h>

/* PR middle-end/38422 */

struct S
{
  int s : (sizeof (int) * CHAR_BIT - 2);
} s;

void
foo (void)
{
  s.s *= 2;
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pic16
  s.s = 24;
  foo ();
  if (s.s != 48)
    ASSERT (0);
  return;
#endif
}
