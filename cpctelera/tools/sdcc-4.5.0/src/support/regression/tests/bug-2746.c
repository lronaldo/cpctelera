/*
   bug-2746.c
*/

#include <testfwk.h>

#include <stdint.h>

struct object_t
{
	uint8_t field1;
	uint8_t field2;
};

void testBug(void)
{
#ifndef __SDCC // Enable when the bug gets fixed!
	(intptr_t)(&(((struct object_t*)0)->field2));
#endif
}

