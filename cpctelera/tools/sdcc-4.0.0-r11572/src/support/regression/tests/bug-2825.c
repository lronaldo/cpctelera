/*
   bug-2825.c a bug in s08 code generation that overwrote a in left shifts of 32-bit values by literals in the range [3,7];
 */

#include <testfwk.h>

unsigned char c;
unsigned long l;

void f(void)
{
	unsigned char t = c + 1; // t allocated to a

	l <<= 6; // Overwrites a.

	if(t != 1)
		f();
}

void testBug(void)
{
	c = 0;
	l = 1;
	f();

	ASSERT(l == 1ul << 6);
}

