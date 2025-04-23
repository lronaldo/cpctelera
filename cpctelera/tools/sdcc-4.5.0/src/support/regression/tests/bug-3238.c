/*
   bug-3238.c. Spilt register parameters were not correctly checked for having their address taken
   resulting in incorrect tail call optimization, thus calls where spilt local variables in the calle
   overwrote the spilt register parameter.
 */

#include <testfwk.h>

#include <string.h>

int *c;

void bar(void);

void foo(int a) __z88dk_fastcall
{
  c = &a;
  bar();
}

void check(char *buffer)
{
	ASSERT (*c == 23);
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


