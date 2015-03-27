/*
   bug-2175.c
*/

#include <testfwk.h>

void f(unsigned char u)
{
    volatile unsigned char i = (unsigned char)((unsigned char)1 << (unsigned char)((unsigned char)u & (unsigned char)0xf));

    ASSERT (i == 0x20);
}

void g(unsigned char u)
{
    volatile unsigned char i = (unsigned char)((unsigned char)0x20 >> (unsigned char)((unsigned char)u & (unsigned char)0xf));

    ASSERT (i == 1);
}

void testBug(void)
{
	f(0x5);
	g(0x5);
}

