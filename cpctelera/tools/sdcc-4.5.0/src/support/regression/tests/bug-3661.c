/* bug-3661.c
   An issue in arithmetic in the Rabbit assembler.
*/

#include <testfwk.h>

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka) || defined(__SDCC_sm83)
unsigned char f(void) __naked __sdcccall(1)
{
__asm
	ld	a, #(0x8000 >> 15)
	ret
__endasm;
}
#else
unsigned char f(void)
{
	return(1);
}
#endif

void
testBug (void)
{
	ASSERT (f() == 1);
}

