/*
   bug-3230.c
   A bug in code generation for __z88dk_callee __critical functions with parameters.
 */

#include <testfwk.h>

#if defined(__SDCC_mcs51) || defined(__SDCC_z80) || defined(__SDCC_z80n) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka) || defined(__SDCC_hc08) || defined(__SDCC_s08) || defined(__SDCC_tlcs90) || defined(__SDCC_stm8) || defined(__SDCC_f8)
volatile int j;

void f0(int i) __z88dk_callee __critical
{
	j = i;
}

int f1(int i) __z88dk_callee __critical
{
	return (i + j);
}
#endif

void testBug(void)
{
#if defined(__SDCC_mcs51) || defined(__SDCC_z80) || defined(__SDCC_z80n) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka) || defined(__SDCC_hc08) || defined(__SDCC_s08) || defined(__SDCC_tlcs90) || defined(__SDCC_stm8) || defined(__SDCC_f8)
	f0(23);
	ASSERT (j == 23);
	ASSERT (f1(23) == 46);
#endif
}

