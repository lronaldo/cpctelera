/*
   bug-2548.c
*/

#include <testfwk.h>

int *p;

void testBug(void)
{
	!(((p != 0)&1));
}

