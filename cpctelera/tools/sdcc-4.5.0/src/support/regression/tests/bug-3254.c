/*
   bug-3254.c. A bug in _Generic handling of implicitly assigned intrinsic named address spaces.
 */

#include <testfwk.h>

int i;

void f(void)
{
    i = _Generic(&i, int * : 1, long * : 2); // Failed to compile this line due to &i being __near int *.
}


void testBug(void)
{
	f();
    ASSERT(i == 1);
}

