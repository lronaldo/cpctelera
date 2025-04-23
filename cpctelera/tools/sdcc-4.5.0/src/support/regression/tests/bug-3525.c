/* bug-3523.c
   Instructions that conditionally skip the following instruction were not considered correctly in the peephole optimizer.
 */

#include <testfwk.h>

char c;

char f(void)
{
	char a = c + 1;
	if (a < 3)
		return 1;
	return a; // The ret got incorrectly optimized out for pdk.
}

void
testBug(void)
{
	c = 3;
	ASSERT (f() == 4);
}

