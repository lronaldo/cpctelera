/*
   bug-2645.c
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 154
#pragma disable_warning 88
#endif

unsigned char *backgrounds[1] = {25768 + 32 + 32 + 32 + 32 + 32 + 32};

void testBug(void)
{
	ASSERT(backgrounds[0] == (unsigned char *)(25768 + 32 + 32 + 32 + 32 + 32 + 32));
}

