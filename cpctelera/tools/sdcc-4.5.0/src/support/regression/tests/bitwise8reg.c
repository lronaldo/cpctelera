/** Test the 8-bit bitwise operators under register pressure
    Meant to trigger use of 16-bit bitwise instructions for 8-bit operations.

 */

#include <testfwk.h>

unsigned char c0, c1, c2, c3;

unsigned char and8(void)
{
	unsigned char t0, t1, t2, t3, t4, t5;
	
	t0 = c0 + 1;
	t1 = c1 + 1;
	t2 = c2 + 1;
	t3 = c3 + 1;
	
	t4 = t0 & t1;
	t5 = t2 & t3;
	
	return(t4 + t5);
}

unsigned char or8(void)
{
	unsigned char t0, t1, t2, t3, t4, t5;
	
	t0 = c0 + 1;
	t1 = c1 + 1;
	t2 = c2 + 1;
	t3 = c3 + 1;
	
	t4 = t0 | t1;
	t5 = t2 | t3;
	
	return(t4 + t5);
}

void
test8(void)
{
	c0 = 2;
	c1 = 2;
	c2 = 4;
	c3 = 4;
	
	ASSERT(and8() == 8);
	ASSERT(or8() == 8);
}

