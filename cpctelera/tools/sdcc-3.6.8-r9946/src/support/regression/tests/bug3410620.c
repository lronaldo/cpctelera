/*
   bug3410620.c
   bank: 1, 2
*/

#include <testfwk.h>

#define INTREGBANK {bank}

#if defined(__SDCC_mcs51) || defined (__SDCC_ds390)

void handler1(void) __using(INTREGBANK)
{
}

typedef void (*phandler_t)(void) __using(INTREGBANK);
phandler_t phandlers[8] = {handler1};

volatile unsigned char GetInterruptSource; //some dummy for real hardware

/* No need to call this, threw
   warning 139: call via function pointer in ISR using non-zero register bank.
   Cannot determine which register bank to save.*/
void Int0_isr(void) __interrupt(0) __using(1)
{
	unsigned char intsrc;
	intsrc = GetInterruptSource;
	phandlers[intsrc](); // warning 139 refers to this line
}

#endif

void testBug(void)
{
	ASSERT (1);
}
