/*
   bug-2601.c
   mem: __idata, __xdata,
*/

#include <testfwk.h>

unsigned char {mem} *p;
unsigned char {mem} *a;
signed char b;

void f(void)
{
    p = a + b;
}

void g(void)
{
    p = b + a;
}

unsigned char {mem} c[2] = {23, 42};

void testBug(void)
{
	a = c + 1;
	b = -1;

	p = 0;
	f();
	ASSERT(*p == 23);

	p = 0;
	g();
	ASSERT(*p == 23);
}
