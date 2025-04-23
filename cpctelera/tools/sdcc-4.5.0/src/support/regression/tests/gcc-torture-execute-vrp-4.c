/*
vrp-4.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

void test(int x, int y)
{
	int c;

	if (x == 1) ASSERT(0);
	if (y == 1) ASSERT(0);

	c = x / y;

	if (c != 1) ASSERT(0);
}

void
testTortureExecute (void)
{
	test(2, 2);
	return;
}
