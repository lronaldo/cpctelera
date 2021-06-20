/* bug-905492.c

   the standard guarantees left-to-right evaluation,
   if the first operand is unequal 0 (resp. 0), the second isn't evaluated.
*/

#include <testfwk.h>

char a;

char
inc_a(char c)
{
  a += 1;
  return c;
}

void
testLeftRightAndOr(void)
{
  volatile char c;

  a = 0; c = inc_a(0) || inc_a(0); ASSERT(a == 2);
  a = 0; c = inc_a(0) || inc_a(1); ASSERT(a == 2);
  a = 0; c = inc_a(1) || inc_a(0); ASSERT(a == 1);
  a = 0; c = inc_a(1) || inc_a(1); ASSERT(a == 1);

  a = 0; c = inc_a(0) && inc_a(0); ASSERT(a == 1);
  a = 0; c = inc_a(0) && inc_a(1); ASSERT(a == 1);
  a = 0; c = inc_a(1) && inc_a(0); ASSERT(a == 2);
  a = 0; c = inc_a(1) && inc_a(1); ASSERT(a == 2);
}
