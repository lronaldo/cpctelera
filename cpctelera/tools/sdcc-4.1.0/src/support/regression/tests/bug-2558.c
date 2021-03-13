/* bug-2807.c
   Sign of % operand lost in redundancy elimination.
 */

#include <testfwk.h>

#include <stdint.h>

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Bug #2874
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
#endif

void testBug(void)
{
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Bug #2874
  x = (unsigned)(-1);
  foo();
  ASSERT (x == (unsigned)(-23) % (unsigned)(-23));
#endif
}

