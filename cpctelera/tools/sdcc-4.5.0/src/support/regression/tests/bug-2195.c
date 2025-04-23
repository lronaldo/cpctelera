/*
   bug1-2195.c

   comparison in foo() destroyed value of child due to incorrect register survival tracking
*/

#include <testfwk.h>

int g(void)
{
	static int i = -2;
	return(i++);
}

void bar(int i)
{
	ASSERT(i == -1);
}

extern void baa(int i)
{
	ASSERT(i != -1);
}

void
foo(void)
{
    int child;

    child = g();

    if (child == -1) {
        bar(child);
    } else {
        baa(child);
    }
}

void testBug(void)
{
	foo();
	foo();
	foo();
	foo();
	foo();
}

