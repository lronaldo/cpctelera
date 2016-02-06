/*
   bug1745717.c
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>

#ifdef __bool_true_false_are_defined

bool and1(char arg)
{
	return arg & 1;
}

bool and2(char arg)
{
	return 1 & arg;
}

#endif //__bool_true_false_are_defined


void
testBug(void)
{
#ifdef __bool_true_false_are_defined
	ASSERT(!and1(0x00));
	ASSERT( and1(0x01));
	ASSERT(!and2(0x00));
	ASSERT( and2(0x01));
#endif //__bool_true_false_are_defined
}
