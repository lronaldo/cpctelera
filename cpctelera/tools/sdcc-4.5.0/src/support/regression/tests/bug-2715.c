/*
   bug-2715.c
*/

#include <testfwk.h>

#pragma disable_warning 85

struct foo;
typedef struct foo FILE;

void pc(FILE *fp, char c)
{
}

void pd(FILE *fp, int n)
{
	int c;

	c = n%10 + '0'; // This addition has operands smaller than the result, resulting in a crash in the z80 backend.
	if (n != 0)
		pd(fp, n);
	pc(fp, c);
}

void testBug(void)
{
	pd(0, 0);
}

