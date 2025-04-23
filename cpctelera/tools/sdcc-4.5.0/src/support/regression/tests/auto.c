/*
   C2X auto
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c23

char c = 7;
auto i = 7;

#endif

void
testAuto(void)
{
#ifdef __SDCC
	static auto s = 7;
	auto j = c + 3;
	ASSERT (sizeof (s) == sizeof (int));
	ASSERT (sizeof (i) == sizeof (int));
	ASSERT (sizeof (j) == sizeof (int));
#endif
}

