/* bug-3661.c
   A crash in code generation for sm83/z80 bitwise operations when both operands are in I/O space.
*/

#include <testfwk.h>

#if defined(__SDCC_z80) || defined(__SDCC_pdk14) || defined(__SDCC_pdk15)
__sfr __at(0x10) changeBase;
__sfr __at(0x11) LCDC_REG;
#elif defined(__SDCC_sm83)
__sfr __at(0xff10) changeBase;
__sfr __at(0xff11) LCDC_REG;
#else
unsigned char changeBase;
unsigned char LCDC_REG;
#endif

void f(void)
{
    LCDC_REG ^= changeBase;
}

void g(void)
{
    LCDC_REG |= changeBase;
}

void h(void)
{
    LCDC_REG &= changeBase;
}

void
testBug (void)
{
}

