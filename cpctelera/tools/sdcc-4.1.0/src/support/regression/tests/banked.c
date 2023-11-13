/** banked.c
*/
#include <testfwk.h>

#if defined(__SDCC_z80) || defined(__SDCC_z80n) || defined(__SDCC_z180) /*|| defined(__SDCC_gbz80)*/ || defined(__SDCC_ez80_z80)
#define DO_CHECK
#endif

static unsigned char bank;
#ifdef DO_CHECK
unsigned c_ab(unsigned a, unsigned b) __banked;
unsigned f_ab(unsigned a) __banked __z88dk_fastcall;
unsigned c_51(unsigned a, unsigned b) __banked;
unsigned f_51(unsigned a) __banked __z88dk_fastcall;
#endif

void
testBanked(void)
{
    ASSERT (bank == 0);
#ifdef DO_CHECK
    ASSERT (c_ab(0x0e, 0x4) == 0xabe4);
    ASSERT (bank == 0);
    ASSERT (f_ab(0x4e) == 0xab4e);
    ASSERT (bank == 0);
    ASSERT (c_51(0x02, 0xd) == 0x51d2);
    ASSERT (bank == 0);
    ASSERT (f_51(0x2d) == 0x2d51);
    ASSERT (bank == 0);
#endif
}


#ifdef DO_CHECK
void set_bank(void) __naked
{
    __asm
set_bank::
	ld	(_bank), a
	ret
    __endasm;
}
void get_bank(void) __naked
{
    __asm
get_bank::
	ld	a, (_bank)
	ret
    __endasm;
}
#pragma bank 0xab
unsigned c_ab(unsigned a, unsigned b) __banked
{
  return bank * 0x100 + a * 16 + b;
}
unsigned f_ab(unsigned a) __banked __z88dk_fastcall
{
  return bank * 0x100 + a;
}
#pragma bank 0x51
unsigned c_51(unsigned a, unsigned b) __banked
{
  return bank * 0x100 + b * 16 + a;
}
unsigned f_51(unsigned a) __banked __z88dk_fastcall
{
  return bank + a * 0x100;
}
#endif
