/*
   bug2956580.c
 */

#include <testfwk.h>

// no need to call this, it generates compiler error:
//   error 33: Attempt to assign value to a constant variable (=)
char foo(const char* s)
{
	char a[1];
	const char* ss = s;
	// The value of the constant variable 's' is not modified rather the pointer is incremented for which SDCC seems to throw error.
	// In reality it was the iTemp for *++s that was considered const but needs to be assigned.
	a[*++s] |= 1;
	return 0;
}

void
testBug (void)
{
	ASSERT (1);
}
