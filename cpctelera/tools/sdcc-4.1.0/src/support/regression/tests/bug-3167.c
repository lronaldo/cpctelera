/*
   bug-3167.c
   a crash in z80 code generation on __sfr right subtraction operand.
 */

#include <testfwk.h>

#if !defined(PORT_HOST) && !defined(__SDCC_stm8) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_tlcs90)
unsigned char effect_y_line;
volatile __sfr __at(0x02) reg_SCY;
volatile __sfr __at(0x04) reg_LY;

void f(void) {
    effect_y_line = 10;
    reg_SCY = effect_y_line - reg_LY;
}
#endif

void
testBug(void)
{
}

