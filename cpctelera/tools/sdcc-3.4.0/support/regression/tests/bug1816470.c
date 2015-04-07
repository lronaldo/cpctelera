/*
    bug 1816470
*/

#include <testfwk.h>

int CalculatedConst(void)
{
	return (unsigned char)(0x100 - 4000000 / 307200.0 + .5);
}

void
testConst(void)
{
	ASSERT(CalculatedConst() == 0xF3);
}
