/*
   bug3482217.c
*/

#include <testfwk.h>

unsigned char *p;

void f()
{
  *p++ = 0;
}

void testBug(void)
{
	unsigned char a[2];
	p = a;
	f();
	ASSERT(p == a + 1);
}

