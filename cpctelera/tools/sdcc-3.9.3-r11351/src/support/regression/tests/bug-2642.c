/*
   bug-2342.c
 */

#include <testfwk.h>

#define VIO (*((volatile unsigned char *) 0x400))

void testBug(void)
{
#if defined(__SDCC_stm8) // only enabled for stm8, since 0x400 maybe a dangerous place on other ports
	VIO = 0xff;
	VIO &= 0x7f;
	VIO &= 0xf7;
	ASSERT (VIO == 0x77);
	VIO |= 0x80;
	VIO |= 0x08;
	ASSERT (VIO == 0xff);
	VIO ^= 0x80;
	VIO ^= 0x08;
	ASSERT (VIO == 0x77);
#endif
}
