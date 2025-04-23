/*
   bug-3282.c. On stm8, the register allocator decided that one byte of a stack-pointer-relative address
   should be rematerialized, and the other stored in a register. stm8 code generation can't handle that yet.
 */

#include <testfwk.h>

void g1(void);

void f1(void)
{
	_Bool i;
	if (&i)
		g1();
}

void g2(void);

void f2(void)
{
	_Bool i;
	_Bool *volatile p = &i;
	if (p)
		g2();
}

void
testBug (void)
{
	f1();
	f2();
}

int g;

void g1(void)
{
	ASSERT(++g == 1);
}

void g2(void)
{
	ASSERT(++g == 2);
}

