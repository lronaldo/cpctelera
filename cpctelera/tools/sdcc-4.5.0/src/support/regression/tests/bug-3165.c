/*
   bug-3165.c
   a crash in z80 code generation on __sfr right subtraction operand.
 */

#include <testfwk.h>

#if !defined(PORT_HOST) && !defined(__SDCC_stm8) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) && !defined(__SDCC_tlcs90) && !defined(__SDCC_f8)

unsigned char effect_y_line;
#if defined(__SDCC_sm83)
volatile __sfr __at(0xff02) reg_SCY;
volatile __sfr __at(0xff04) reg_LY;
#elif defined(__SDCC_mcs51) || defined(__SDCC_ds390)
volatile __sfr __at(0x82) reg_SCY;
volatile __sfr __at(0x84) reg_LY;
#else
volatile __sfr __at(0x02) reg_SCY;
volatile __sfr __at(0x04) reg_LY;
#endif

void f(void) {
    effect_y_line = 10;
    reg_SCY = effect_y_line - reg_LY;
}
#endif

void
testBug(void)
{
}

