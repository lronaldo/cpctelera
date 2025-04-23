/*
   bug-2645.c - conversion from integers to pointers was broken in initalizations.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 154
#pragma disable_warning 88

unsigned char *backgrounds[1] = {25768 + 32 + 32 + 32 + 32 + 32 + 32};
#endif

void testBug(void)
{
#ifdef __SDCC
	ASSERT(backgrounds[0] == (unsigned char *)(25768 + 32 + 32 + 32 + 32 + 32 + 32));
#endif
}

