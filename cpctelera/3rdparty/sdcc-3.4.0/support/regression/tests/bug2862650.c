/*
 * bug2862650.c
 */

#include <testfwk.h>

void
testBug(void)
{
	static const float a[] = {1.5, 2.5, 3.5};
	const float b[] = {4.5, 5.5, 6.5};
	static float c[] = {7.5, 8.5, 9.5};
	volatile char i;

	i = 1;

	ASSERT(a[i] == 2.5);
	ASSERT(b[i] == 5.5);
	ASSERT(c[i] == 8.5);
}

