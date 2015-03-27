/*
   bug2942247.c
*/

#include <testfwk.h>


#pragma std_sdcc99

#ifndef __SDCC_WEIRD_BOOL
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
#endif

void testBug(void)
{
#ifndef __SDCC_WEIRD_BOOL
	ASSERT(bar() == 1);
	ASSERT(bar2() == 1);
#endif
}

