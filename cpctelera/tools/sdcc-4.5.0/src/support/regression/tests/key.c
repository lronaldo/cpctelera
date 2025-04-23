/* key.c - a ward to alert us in case obsolete sdcc-specific keywords rise from the dead.
 */

#include <testfwk.h>

#if !defined(__llvm__) && !defined(__clang__)
int _asm;
#endif

#ifndef __SDCC_pdk14 // Lack of memory
int _endasm, at, bit, code, critical, data, far,
eeprom, fixed16x16, flash, idata, interrupt, nonbanked, banked, near,
pdata, reentrant, shadowregs, wparam, sfr, sfr16, sfr32, sbit, sram,
using, _naked, xdata, _overlay;
#endif

void testBug(void)
{
  ASSERT(1);
}
