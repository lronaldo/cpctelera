/*
	bug 1714204
*/

#include <testfwk.h>

typedef struct {
	unsigned int w[2];
} TEST_TYPE;

unsigned char i1, i2, i3;

void foo(TEST_TYPE *p1, TEST_TYPE *p2, TEST_TYPE *p3)
{
	if (p2->w[i2] > ++p3->w[i3])
		p1->w[i1] = p2->w[i2] + p3->w[i3];
}

void
testBug(void)
{
	TEST_TYPE t1 = { { 1, 1 } };
	TEST_TYPE t2 = { { 1, 1 } };
	TEST_TYPE t3 = { { 1, 1 } };

	i1 = i2 = i3 = 0;

	foo(&t1, &t2, &t3);

	ASSERT (t1.w[0] == 1);
}
