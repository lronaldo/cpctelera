/*
   bug-2964.c

   A bug in stm8 code generation for left shift triggered by stack allocation allocating
   result and right operands of a left shift to the stack, to overlapping locations.
*/

#include <testfwk.h>

unsigned long long j;

unsigned char c;
unsigned long long i;

// Just some function that won't be inlined, so calls to it will result in spilling of local variables.
void g(void)
{
	j += 42 + i;
}

unsigned long long f1(void)
{
	unsigned char d = c + 1;
	j |= 7;
	unsigned long long l1 = i + 1;
	g();
	g();
	unsigned long long l2 = l1 << d;

	return l2;
}

#ifndef __SDCC_pdk14 // Lack of RAM
unsigned long long f2(void)
{
	unsigned char d = c + 1;
	j |= 7;
	unsigned long long l1 = i + 1;
	g();
	g();
	g();
	g();
	unsigned long long l2 = l1 << d;

	return l2;
}

unsigned long long f3(void)
{
	unsigned char d = c + 1;
	g();
	g();
	unsigned long long l2 = i << d;

	return l2;
}
#endif

void testBug(void)
{
	j = 1;

	c = 0;
	i = 1;
	ASSERT(f1() == 4);

#ifndef __SDCC_pdk14 // Lack of RAM
	c = 1;
	i = 3;
	ASSERT(f2() == 16);

	c = 1;
	i = 8;
	ASSERT(f3() == 32);
#endif
}

