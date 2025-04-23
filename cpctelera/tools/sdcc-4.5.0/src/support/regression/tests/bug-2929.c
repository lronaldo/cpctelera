/*
   bug-2929.c

   A bug in z80 code generation for ~.
*/

#include <testfwk.h>

volatile unsigned char c;
unsigned int i;

void f(void)
{
	unsigned int j = (unsigned char)(c + 1);
	i = ~j | 0xaa55;
}

void testBug(void)
{
	c = 0xff;
	f();
	ASSERT((i & 0xffffu) == 0xffffu);

	c = 0x00;
	f();
	ASSERT((i & 0xffffu) == 0xffffu);

	c = 0xfe;
	f();
	ASSERT((i & 0xffffu) == 0xff55u);
}

