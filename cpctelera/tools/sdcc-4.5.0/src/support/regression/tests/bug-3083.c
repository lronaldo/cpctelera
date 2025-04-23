/* bug-3083.c
   Invalid asm generated for bitwise or of variable allocated to a with sfr.
 */

#include <testfwk.h>

#include <stdint.h>

#if defined (__SDCC_pdk14) || defined (__SDCC_pdk15)
__sfr __at(0x0c) _integs;
#else
unsigned char _integs;
#endif

#define INTEGS _integs

struct {
  uint8_t val;
} my_struct;

volatile uint8_t val = 123;

void f(void)
{
  INTEGS |= val; // works
  INTEGS |= my_struct.val; // error
}

void
testBug(void)
{
}

