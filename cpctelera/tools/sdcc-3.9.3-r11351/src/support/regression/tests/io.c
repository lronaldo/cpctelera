/* io.c
   Some source code extracted from the ColecoVision / Sega 8-bit game Io.
   This file has been included in the tests, since it uses some z80 peephole rules
   not used elsewhere in the regression tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdint.h>
#include <stdbool.h>

void f16(uint16_t i)
{
	ASSERT(i == 0x55aa);
}

void f16_2(uint16_t i)
{
	ASSERT(i == 0xaa55);
}

void f8(uint8_t i)
{
	ASSERT(i == 0x5a);
}

uint16_t assemble(uint8_t i, uint8_t j)
{
	return(j | ((uint16_t)i << 8));
}

#ifndef __SDCC_pdk14 // Lack of memory
// Sequence of function calls with stack parameters close to stack end.
void callsequencestack(uint16_t i, uint16_t j, uint8_t k)
{
	f16(i);
	f16_2(j);
	f8(k);
	f16(i);
}

// Sequence of function calls with stack parameters far from stack end.
void callsequencestack2(uint32_t dummy1, uint16_t i, uint16_t j, uint8_t k, uint32_t dummy2)
{
	dummy1;
	dummy2;
	f16(i);
	f16_2(j);
	f8(k);
	f16(i);
}

// Sequence of function calls with stack parameters far from stack end.
void callsequencestack3(uint32_t dummy1, uint8_t i, uint8_t j, uint8_t k, uint32_t dummy2)
{
	callsequencestack(assemble(i, j), assemble(j, i), k);
	callsequencestack2(dummy1, assemble(i, j), assemble(j, i), k, dummy2);
}

// Sequence of function calls with with literals parameters
void callsequencelit(void)
{
	f16(0x55aa);
	f16_2(0xaa55);
	f8(0x5a);
	f16(0x55aa);
	callsequencestack(0x55aa, 0xaa55, 0x5a);
	callsequencestack2(0, 0x55aa, 0xaa55, 0x5a, 0);
	callsequencestack(assemble(0x55,0xaa), assemble(0xaa,0x55), 0x5a);
	callsequencestack3(0, 0x55, 0xaa, 0x5a, 0);
}
#endif

struct flags
{
	uint8_t a;
	uint8_t b;
};

uint8_t *c;

void setflags(struct flags *f)
{
	f->a = 0x55;
	f->b = 0xaa;
	c = &(f->b);;
}

// Test a bit in a location accessed stough struct / by pointer
void pointerbit(void)
{
	struct flags f;

	setflags(&f);

	if(f.a & 8)
		ASSERT(0);
	if(!(f.a & 4))
		ASSERT(0);
	if(f.b & 4)
		ASSERT(0);
	if(!(f.b & 8))
		ASSERT(0);
	if(*c & 4)
		ASSERT(0);
	if(!(*c & 8))
		ASSERT(0);
}

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
	callsequencelit();
#endif
	pointerbit();
}

