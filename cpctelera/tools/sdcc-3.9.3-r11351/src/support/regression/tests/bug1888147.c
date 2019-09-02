/*
    bug 1888147
*/

#include <testfwk.h>

// no need to call this, it generates compiler error:
//   Caught signal 11: SIGSEGV
int foo(int n)
{
	int i = 0;

	if (i!=0)
		return n;
	return 0;
}

void
testBug(void)
{
	ASSERT(1);
}
