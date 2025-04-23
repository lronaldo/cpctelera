/*
packed-2.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

typedef struct s {
	unsigned short a;
	unsigned long b;
} s;

s t;

void
testTortureExecute (void)
{
        t.b = 0;
	return;
}
