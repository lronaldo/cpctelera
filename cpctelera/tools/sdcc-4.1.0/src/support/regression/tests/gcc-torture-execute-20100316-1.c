/*
   20100316-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#ifndef __SDCC_pic16 // TODO: enable when the pic16 ports supports bitfields of size greater than 8 bits!
struct Foo {
  int i;
  unsigned precision : 10;
  unsigned blah : 3;
} f;

int
foo (struct Foo *p)
{
  struct Foo *q = p;
  return (*q).precision;
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pic16
  f.i = -1;
  f.precision = 0;
  f.blah = -1;
  if (foo (&f) != 0)
    ASSERT (0);
  return;
#endif
}

