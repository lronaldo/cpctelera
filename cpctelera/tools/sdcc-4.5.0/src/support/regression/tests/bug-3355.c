/* bug-3255.c
   A bug in comparisons on mcs51.
 */
 
#include <testfwk.h>

volatile unsigned char ua, ub;

int f(void)
{
	if((ua <= ub) == 1) // No bug without the == 1 here.
		return 1;
	return 0;
}

void testBug(void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) // Enable when bug is fixed.
	ua = 1;
	ub = 2;
	ASSERT (f());
#endif
}

