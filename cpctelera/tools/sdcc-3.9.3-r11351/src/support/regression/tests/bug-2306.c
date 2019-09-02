/*
   bug-2306.c frame pointer omission triggered a code generation bug in 16-bit addition short functions.
 */

#include <testfwk.h>

int add(int pa, int pb)
{
	return pa * 2 + pb;
}

void testBug(void)
{
	ASSERT (add(0x0023, 0x0042) == 0x0023 * 2 + 0x0042);
	ASSERT (add(0x2300, 0x4200) == 0x2300 * 2 + 0x4200);
}

