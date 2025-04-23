/*
   bug-2752.c
   A bug in inlining of reentrant functions into nonreentrant functions in the mcs51 backend.
 */

#include <testfwk.h>

#include <stdint.h>

inline uint8_t test(uintptr_t addr) __reentrant
{
        return *(volatile uint8_t __xdata *)(addr);
}

__xdata uint8_t dat1;
uint8_t dat2;

void call(void) /* Unbalanced stack pointer in this function */
{
	dat2 = test((uintptr_t)(&dat1));
}
	
void testBug(void)
{
	dat1 = 0x5a;
	call();
	ASSERT (dat2 == 0x5a);
}

extern uint8_t test(uintptr_t addr) __reentrant;

