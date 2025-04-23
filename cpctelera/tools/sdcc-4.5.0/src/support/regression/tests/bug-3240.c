/*
   bug-3240.c. Stack register parameters were not correctly checked for having their address taken
   resulting in incorrect tail call optimization, thus calls where spilt local variables in the calle
   overwrote the spilt register parameter.
 */

#include <testfwk.h>

#include <string.h>

void bar(void);

int *p;

void foo(int a) __z88dk_callee
{
	p = &a;
	bar();
}

void check(char *buffer)
{
	ASSERT (*p == 23);
	ASSERT (buffer[0] == 42);
}

void bar(void)
{
	char buffer[8];
	memset(buffer, 42, 8);
	check(buffer);
}

void
testBug (void)
{
  foo(23);
  return;
}

