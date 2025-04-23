/* bug-3711.c
   A bug in compile-time conversion of large floating-point constants, on host systems with a 32-bit long.
*/

#include <stdint.h>

#include <testfwk.h>

const uint64_t f=1e18; // floating-point constant converted to unsigned integer.
uint64_t i=1000000000000000000;

void
testBug (void)
{
  ASSERT (i * 2 >= f && f >= i / 2);
}

