/*
   bug1-2195.c

   Assertion failure in z80 code generation.
*/

#include <testfwk.h>

#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined (__SDCC_mos6502) && !defined(__SDCC_mos65c02) && !defined(__SDCC_pdk14) && !defined (__SDCC_pdk15) /* mcs51, hc08, s08 and pdk14 have restrictions on function pointers wrt. reentrancy */
void f(void)
{
	((void (*)(int)) 0)(0);
}
#endif

void testBug(void)
{
}

