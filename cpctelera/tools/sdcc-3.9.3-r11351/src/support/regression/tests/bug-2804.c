/*
    bug-2804.c
*/

#include <testfwk.h>

#include <limits.h>
#include <stdbool.h>

volatile unsigned long long a;

unsigned long long f(_Bool c)
{
	if(c)
		return(a + (ULLONG_MAX - 2));
	else
		return(a + (ULLONG_MAX - 3));
}

void testBug(void)
{
	a = 0;
	ASSERT(f(true) != f(false));
}

