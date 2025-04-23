/* bug-3522.c
   A bug in insertion of support function calls for multiplicative operators for _BitInt where one operand was a literal.
 */

#include <testfwk.h>

#include <stdint.h>

#if __SDCC_BITINT_MAXWIDTH >= 32 // TODO: When we can regression-test in --std-c23 mode, use the standard macro from limits.h instead!
typedef _BitInt(32) data_t;
#else
typedef int32_t data_t; // this one worked
#endif

data_t foo(int val) {
  return (data_t)val/2; // Compiler internal error on this line for _BitInt.
}

void
testBug (void)
{
  ASSERT (foo (42) == 21);
}

