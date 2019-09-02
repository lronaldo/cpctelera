/*
   bug-2645.c
 */

#include <testfwk.h>

#pragma disable_warning 88

unsigned char *backgrounds[1] = {25768 + 32 + 32 + 32 + 32 + 32 + 32};

void testBug(void)
{
	ASSERT(backgrounds[0] == (unsigned char *)(25768 + 32 + 32 + 32 + 32 + 32 + 32));
}

