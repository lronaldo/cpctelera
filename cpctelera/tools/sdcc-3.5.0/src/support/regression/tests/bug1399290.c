/* bug1399290.c
 */
#include <testfwk.h>

unsigned long Left = 0x12345678;

void
testLongPlus(void)
{
	static unsigned long Result;
	static unsigned long Rhs = 0x87654321;
	static unsigned long *Lhs = &Left;

	Result = *Lhs + Rhs;
	ASSERT (Result == 0x99999999);
}
