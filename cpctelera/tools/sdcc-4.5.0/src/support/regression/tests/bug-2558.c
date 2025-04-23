/* bug-2807.c
   Sign of % operand lost in redundancy elimination.
 */

#include <testfwk.h>

#include <stdint.h>

uint16_t x;

static inline uint16_t llvm_srem_u16(int16_t a, int16_t b) {
  uint16_t r = a % b;
  return r;
}

static inline uint16_t returnx(void)
{
  return x;
}

void foo(void)
{
  x=llvm_srem_u16(x,returnx());
}

void testBug(void)
{
  x = (unsigned)(-1);
  foo();
  ASSERT (x == (unsigned)(-23) % (unsigned)(-23));
}

