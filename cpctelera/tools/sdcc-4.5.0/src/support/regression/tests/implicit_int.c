/*
   The C90 implicit int rule
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c89
#pragma disable_warning 85
#pragma disable_warning 225

static x;
volatile y;
#endif

void testint(void)
{
#ifdef __SDCC // Don't know how to set host compiler to C90 mode.
	auto a;
	static b;
	register c;
	volatile d;
#endif
}

