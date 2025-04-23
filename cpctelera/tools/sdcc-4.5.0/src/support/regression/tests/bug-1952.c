/*
   bug-1952.c - missing __func__
 */

#include <testfwk.h>
#include <string.h>

void ratherquitelongfuntionname(void)
{
	const char *funcp = __func__;
	ASSERT(funcp == __func__);
	ASSERT(!strcmp(__func__, "ratherquitelongfuntionname"));
	ASSERT(_Generic(__func__, const char *: 1, default: 0));
	ASSERT(sizeof(__func__) == strlen("ratherquitelongfuntionname") + 1);
}

void testBug(void)
{
	ratherquitelongfuntionname();
}

