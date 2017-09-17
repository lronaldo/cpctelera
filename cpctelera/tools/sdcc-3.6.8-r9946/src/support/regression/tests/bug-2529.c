/*
    bug 2529
*/

#include <testfwk.h>

int i;

int f(void)
{
	i = 1;
}

void
testConst(void)
{
	f() - f();
	ASSERT(i);
}

