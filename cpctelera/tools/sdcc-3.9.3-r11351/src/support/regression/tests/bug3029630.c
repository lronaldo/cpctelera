/*
   bug3029630.c
 */

#include <testfwk.h>

struct a
{
	void *a;
} s;

/* should not give 
   warning 196: pointer target lost const qualifier */
void foo(const struct a *x)
{
	void *const y = x->a;
	(void)y;
}

void testBug(void)
{
	foo(&s);
}
