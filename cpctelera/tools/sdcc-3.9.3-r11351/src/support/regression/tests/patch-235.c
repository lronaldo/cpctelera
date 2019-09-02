/*
   patch-235.c - used shift insetad of rotate instruction.
 */

#include <testfwk.h>

unsigned long sss(unsigned long a)
{
	return a >> 9;
}

void testBug(void)
{
	ASSERT(sss(0x55555555) == 0x002aaaaa);
}

