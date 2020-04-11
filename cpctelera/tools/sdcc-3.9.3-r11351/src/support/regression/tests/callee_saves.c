/** test callee_saves function calling caller_saves
    see bug 1928
*/

#include <testfwk.h>

#if defined (__mcs51) && !defined (SDCC_MODEL_HUGE)

int x(int a, int b)
{
	return a - b;
}

#pragma callee_saves y
int y(int a, int b)
{
	int c = a + b;
	x(a, b);
	return c;
}

#pragma callee_saves z
void z(void)
{
	x(1, 2);
}

int g = 100;

#endif

void testBug(void)
{
#if defined (__mcs51) && !defined (SDCC_MODEL_HUGE)
	int a = g;
	ASSERT (y(1, 2) == 3);
	ASSERT (a == 100);
	a = g;
	z();
	ASSERT (a == 100);
#endif
}
