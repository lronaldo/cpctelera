/*
   20030218-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/*  On H8, the predicate general_operand_src(op,mode) used to ignore
    mode when op is a (mem (post_inc ...)).  As a result, the pattern
    for extendhisi2 was recognized as extendqisi2.  */

short *q;

long
foo (short *p)
{
  long b = *p;
  q = p + 1;
  return b;
}

void
testTortureExecute (void)
{
  short a = 0xff00;
  if (foo (&a) != (long) (short) 0xff00)
    ASSERT (0);
  return;
}

