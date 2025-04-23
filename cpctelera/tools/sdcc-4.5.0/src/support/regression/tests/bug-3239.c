/*
   bug-3239.c. A jump was incorrectly redirected to a non-exisitng label in the peephole optimizer when the name of the callee was a prefix of the caller's name.
 */
 
#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#ifndef __SDCC_stm8
#define __cosmic
#endif

unsigned char f(unsigned char i) __cosmic;

unsigned char f_wrap_cosmic(unsigned char i) __cosmic
{
	return f(i);
}

void
testBug (void)
{
	ASSERT (f_wrap_cosmic (23) == 42);
}

unsigned char f(unsigned char i) __cosmic
{
	return i == 23 ? 42 : 3;
}

