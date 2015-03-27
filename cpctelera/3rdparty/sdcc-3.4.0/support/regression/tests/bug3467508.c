/*
   bug3467508.c

   Cast of integer literals to pointer behaved differently than cast from integer.
*/

#ifdef __SDCC
#pragma disable_warning 88
#pragma disable_warning 127
#endif

#include <testfwk.h>

void testBug(void)
{
#ifndef __SDCC_pic16
	volatile int i = -1;
	ASSERT((void *)(i) == (void *)(-1));
#endif
}
