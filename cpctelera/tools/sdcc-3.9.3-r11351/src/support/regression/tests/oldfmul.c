/*
   oldfmul.c - a bug that resulted in wrong results in an old float multiplication routine.
*/

#include <testfwk.h>

union float_long
  {
    float f;
    unsigned long l;
  };

#define HIDDEN		(unsigned long)(1ul << 23)

void lmul (unsigned long l, unsigned long r)
{
  ASSERT (l == HIDDEN >> 8);
  ASSERT (r == HIDDEN >> 8);

}

void oldfmul11 (void) {
  volatile union float_long fl1, fl2;

  fl1.l = HIDDEN;
  fl2.l = HIDDEN;

  lmul((fl1.l >> 8), (fl2.l >> 8));
}

void
testBug (void)
{
  oldfmul11 ();
}

