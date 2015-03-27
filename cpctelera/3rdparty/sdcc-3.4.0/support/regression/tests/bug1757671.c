/*
   bug1757671.c
 */

#include <testfwk.h>
#include <stdint.h>

struct c
{
	float r;
};

void clamp(struct c *l)
{
	if(l->r > 240.0f)
		l->r = 240.0f;
	if(l->r < 1.0f)
		l->r = 1.0f;
}

//char cmplong(char x, char y)
char cmplong(int32_t x, int32_t y)
{
	if (x < y)
		return 1;
	return 0;
}

void testBug(void)
{
//	volatile char x = 0xBF;//800000; //-1.0
//	volatile char y = 0x3F;//800000; //+1.0
//	volatile char z = 0x43;//700000; //+240.0
	volatile int32_t x = 0xBF800000; //-1.0
	volatile int32_t y = 0x3F800000; //+1.0
	volatile int32_t z = 0x43700000; //+240.0
	struct c val = { -1.0 };
	clamp(&val);
	ASSERT (val.r == 1.0);
	ASSERT (x <= y);
	ASSERT (cmplong(x, y));
	ASSERT (x < z);
	ASSERT (cmplong(x, z));
}
