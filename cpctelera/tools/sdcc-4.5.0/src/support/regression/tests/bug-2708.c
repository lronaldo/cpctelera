/* bug-2708.c
   The STM8 peephole optimizer didn't like indirect addressing modes in inline asm.
 */

#include <testfwk.h>
#include <stdint.h>

#ifdef __SDCC
#pragma std_c99
#endif

uint8_t *cp;

void f1(void)
{
	*cp >>= 1;
}

#ifdef __SDCC_stm8

void f2(void)
{

__asm
	srl	[_cp]
__endasm;
}

void f3(void)
{
__asm
	ldw	y, _cp
	srl	(y)
__endasm;
}

void f4(void)
{
__asm
	ld	a, [_cp]
	srl	a
	ld	[_cp], a
__endasm;
}

void f5(void)
{
__asm
	clrw	x
	srl	([_cp], x)
__endasm;
}

#else

#define f2 f1
#define f3 f1
#define f4 f1
#define f5 f1

#endif


void testBug(void)
{
	uint8_t c;
	cp = &c;

	c = 0xaa;
	f1();
	ASSERT(c == 0x55);

	c = 0xaa;
	f2();
	ASSERT(c == 0x55);

	c = 0xaa;
	f3();
	ASSERT(c == 0x55);

	c = 0xaa;
	f4();
	ASSERT(c == 0x55);

	c = 0xaa;
	f5();
	ASSERT(c == 0x55);
}

