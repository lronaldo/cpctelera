/* bug-3085.c
   Pdk code generation for wide < overwrote upper byte of operand in p.
 */
 
#include <testfwk.h>

#include <stdint.h>

uint16_t f(void)
{
  uint16_t cc = 0;
  for (uint16_t j = 0; j < 500; j++) { // Code generation for upper byte for < overwrote upper byte of j.
    cc++;
  }

  cc = ~cc;

  return cc;
}

void
testBug(void)
{
  ASSERT (f() == (uint16_t)(~(uint16_t)500));
}

