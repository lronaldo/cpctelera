/*
   bug3379723.c
*/

#include <testfwk.h>

#if !defined(PORT_HOST) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) && !defined(__SDCC_sm83) && !defined(__SDCC_stm8) && !defined(__SDCC_tlcs90) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_f8)

#define UCHAR	unsigned char

__sfr __at 0xf0 rGPIO_B_Output_Enable;
__sfr __at 0xf1 rGPIO_B_Input_Enable;
__sfr __at 0xf2 rGPIO_B;

UCHAR _Check_B_By_B(UCHAR iAndMask)		// pull low
{
	UCHAR iVal, iOrMask;

	iOrMask = ~iAndMask;
	rGPIO_B_Input_Enable &= iAndMask;
	rGPIO_B_Output_Enable |= iOrMask;	
	rGPIO_B |= iOrMask;
	iVal = rGPIO_B;
	iVal &= iAndMask;
	rGPIO_B &= iAndMask;
	rGPIO_B_Output_Enable &= iAndMask;	
	rGPIO_B_Input_Enable |= iOrMask;

	return iVal;
}
#endif

void
testBug (void)
{
  ASSERT (1);
}

