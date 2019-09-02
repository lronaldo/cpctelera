/*
 * bug3290691.c
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>

bool foo_b(bool b)
{
	return !b;
}

unsigned char bar_c(unsigned char c)
{
	return foo_b(c);
}

unsigned char foo_c(unsigned char c)
{
	return !c;
}

bool bar_b(bool c)
{
	return foo_c(c);
}

void
testBug(void)
{
#ifndef __SDCC_pic16
	ASSERT( bar_c(1) == 0 );
	ASSERT( bar_c(0) == 1 );
	ASSERT( bar_b(1) == 0 );
	ASSERT( bar_b(0) == 1 );
#endif
}
