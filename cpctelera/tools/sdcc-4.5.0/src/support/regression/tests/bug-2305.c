/*
   bug-2305.c register packing optimized away the assignment even though code generation for left shift cannot deal with sfr result operand.
 */

#include <testfwk.h>

#if !defined(PORT_HOST) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) && !defined(__SDCC_sm83) && !defined(__SDCC_stm8) && !defined(__SDCC_tlcs90) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_f8)
__sfr __at 0xF4 fd_select;

static void foo(unsigned char x)
{
	fd_select = 1 << x;
}

static void bar(unsigned char x)
{
	unsigned char a = 1 << x;
	fd_select = a;
}
#endif

void testBug(void)
{
}

