/*
   bug3404101.c
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdbool.h>

struct a
{
	bool a : 1;
};

void f(struct a *s)
{
	1 && s->a;	// Compilation failed: Logical op with bitfield boolean.
}

void testBug(void)
{
}

