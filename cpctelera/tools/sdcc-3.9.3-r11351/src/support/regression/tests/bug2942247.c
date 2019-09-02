/*
   bug2942247.c
*/

#include <testfwk.h>


#pragma std_sdcc99

const _Bool foo[2] = {1, 2};

_Bool bar(void)
{
	return foo[1];	// Returned foo + 1 instead of foo[1].
}

_Bool bar2(void)
{
	_Bool foo[2] = {1, 2};
	return foo[1];	// Crashed sdcc.
}

void testBug(void)
{
	ASSERT(bar() == 1);
	ASSERT(bar2() == 1);
}

