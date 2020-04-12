/** bug1994062.c

    sign: signed, unsigned
 */

#include <testfwk.h>

/*
  8051 code miscompares idata char* pointers.
  It does signed compare instead of unsigned.
  And I offer an optimzation that can't be peepholed, at least not by me.
  2.7.0 and 2.8.0 are identical with respect to this bug. 2.8.0 generates a false warning from the linker.

  Also for xdata it generates error 47: indirections to different types assignment
    from type 'unsigned-char near* '
    to type 'unsigned-char xdata* '
 */

{sign} char _STATMEM *one;
{sign} char _STATMEM *two;
{sign} char chunk[20];

void testBug(void)
{
  one = chunk;
  two = &one[140];
  ASSERT (one < two);
}
