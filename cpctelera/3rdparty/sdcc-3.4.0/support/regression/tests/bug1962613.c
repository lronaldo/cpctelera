/*
   bug1962613.c
 */

#include <testfwk.h>

struct b {
	char b0;
	char b1;
};

struct a {
	const struct b *q[2];
	const char *r[2];
};

const struct b c0[3] = {
	{000, 001}, {010, 011}, {020, 021}
};

const struct b c1[3] = {
	{100, 101}, {110, 111}, {120, 121}
};

const char k0[3] = { 50, 51, 52 };
const char k1[3] = { 60, 61, 62 };

const struct a x = {
	{c0, c1}, {k0, k1}
};

void testBug(void)
{
	ASSERT (x.q[1][2].b1 == 121);
	ASSERT (x.r[1][2] == 62);
}
