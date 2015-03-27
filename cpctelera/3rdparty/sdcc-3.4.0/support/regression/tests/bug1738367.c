/*
   bug1738367.c
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>

#ifdef __bool_true_false_are_defined

bool ternary(unsigned char status)
{
	return (status == 0) ? 0 : 1;
}

bool ternary_inv(unsigned char status)
{
	return (status == 0) ? 1 : 0;
}


bool ternary1(unsigned char status)
{
	return status ? 1 : 0;
}

bool ternary1_inv(unsigned char status)
{
	return status ? 0 : 1;
}


bool ternary2(unsigned char status)
{
	return !status ? 0 : 1;
}

bool ternary2_inv(unsigned char status)
{
	return !status ? 1 : 0;
}

#endif //__bool_true_false_are_defined


void
testBug(void)
{
#ifndef __SDCC_pic16
#ifdef __bool_true_false_are_defined
	ASSERT(!ternary(0x00));
	ASSERT( ternary(0x10));

	ASSERT( ternary_inv(0x00));
	ASSERT(!ternary_inv(0x10));

	ASSERT(!ternary1(0x00));
	ASSERT( ternary1(0x10));

	ASSERT( ternary1_inv(0x00));
	ASSERT(!ternary1_inv(0x10));

	ASSERT(!ternary2(0x00));
	ASSERT( ternary2(0x10));

	ASSERT( ternary2_inv(0x00));
	ASSERT(!ternary2_inv(0x10));
	ASSERT(!ternary2_inv(1==1));
#endif //__bool_true_false_are_defined
#endif
}
