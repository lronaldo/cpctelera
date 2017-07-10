/*
    bug-2221.c
*/

#include <testfwk.h>

#ifdef __SDCC_mcs51
#pragma nooverlay
#include <8052.h>

//two functions that use the same registers (but should be in different banks)

unsigned char calculate(unsigned char v1,unsigned char v2)
{
	unsigned char v3;
	TF2 = 1;			//trigger the interrupt
	v3 = v1 / 2;
	return v3 + v1 + v2;
}


unsigned char calculateISR(unsigned char v1,unsigned char v2) __using(1)
{
	unsigned char v3;
	TF2 = 0;			//clear the interrupt
	v3 = v1 / 2;
	return v3 + v1 + v2;
}


unsigned char r1;
unsigned char r2;

void T2_isr(void) __interrupt(5) __using(1)
{
	r1 = calculateISR(4,40);

	//sdcc eliminates mov psw,0x080
	//which is necessary to ensure that calculateISR uses registers from bank 1 to not
	//corrupt calculate in main loop
}
#endif

void testBug(void)
{
#ifdef __SDCC_mcs51
	TF2 = 0;				//clear timer 2 interrupt
	ET2 = 1;				//enable timer 2 interrupt
	EA = 1;					//enable interrupts

	r2 = calculate(8,30);
	ASSERT(r1 == 46);
	ASSERT(r2 == 42);
#endif
}

