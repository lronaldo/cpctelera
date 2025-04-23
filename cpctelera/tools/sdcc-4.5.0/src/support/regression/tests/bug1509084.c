/*
    bug 1509084
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>

#if !defined(__bool_true_false_are_defined)
#define bool unsigned char
#endif

unsigned char aa, bb, cc, dd, ee;

void leds_name_repaint(void)
{
	unsigned char an;
	unsigned char dg = aa;
	bool s = 0;

	for( an = 0; an < 5; an ++ )
	{
		s = (  (long) aa >> 1 ) > 0;
		if( s )
		{
			aa += dg + 1;
			bb += dg + 2;
			cc += dg + 3;
			dd += dg + 4;
			ee += dg + 5;
		}
	}
}

void
testBug(void)
{
	aa = 2;
	leds_name_repaint();
	ASSERT(aa == 17);
}

