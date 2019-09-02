/*
   The C90 implicit int rule
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c89
#pragma disable_warning 85
#pragma disable_warning 225
#endif

static x;
volatile y;

void testint(void)
{
	auto a;
	static b;
	register c;
	volatile d;
}

