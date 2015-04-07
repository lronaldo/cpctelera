/*	bug-2163.c
*/

#include <testfwk.h>

void foo(void)
{
}

unsigned char a;

/* No need to run, generated error 56: Duplicate label '_iftrue_0' */
int bug2163(void)
{
    if (a)
		foo();
iftrue_0:
_iftrue_0:
    return 0;
}

void bar(const char* s)
{
	s;
}

char str_0[] = "mystring";
char _str_0[] = "mystring";

void testBug(void)
{
	/* creates string constant which gave
	   error 177: Duplicate symbol '_str_0', symbol IGNORED */
	bar("test");
	ASSERT(1);
}
