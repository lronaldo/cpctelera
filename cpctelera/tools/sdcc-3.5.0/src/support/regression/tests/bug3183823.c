/*
   bug3183823.c
*/

#include <testfwk.h>

float neg (float x)
{
	return -x;
}

void testNeg (void)
{
#ifndef __SDCC_pic16
	float x, y;
	char a, b;

	x = neg(0.0);
	y = neg(-0.0);
	ASSERT (x==0.0);
	ASSERT (y==0.0);
	ASSERT (x==-0.0);
	ASSERT (y==-0.0);
	ASSERT (x==y);
	ASSERT (x>=y);
	ASSERT (x<=y);
	ASSERT (y>=x);
	ASSERT (y<=x);

	a = ((char*)&x)[0];
	b = ((char*)&x)[3];
	((char*)&x)[0] = b;
	((char*)&x)[3] = a;
	ASSERT (x!=0.0);
#endif
}
