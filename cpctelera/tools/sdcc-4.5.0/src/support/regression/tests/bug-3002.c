/*
   bug-3002.c - a bug in stm8 code generation for right shifts.
 */

#include <testfwk.h>

#include <stdint.h>

uint_least32_t f(uint_least32_t arg)
{
	return((arg >> 22) | 0x15000000); // Operands chosen to encourage allocation of upper byte of shift result in accumulator and use of div for shift.
}
void testBug(void)
{
	ASSERT(f(0xaaa00000) == 0x150002aa);
}

