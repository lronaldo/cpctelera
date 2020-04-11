/*
   bug3190029.c
*/

#include <testfwk.h>

void bar(void)
{
}

/* No need to call this, threw
   error 20: Undefined identifier 'c' */
void foo(void)
{
	{
		int c;
		c = 0;	//undefined identifier?
	}
	(void (*)(void *))bar;
}

void testBug(void)
{
	ASSERT (1);
}
