/*
   bug-3215.c
   A bug in merging sequences of puts
 */

#include <testfwk.h>

#pragma disable_warning 283

#include <stdio.h>

void foo(void);

int
bug ()
{
    puts ("1");
    puts ("2");

    foo();

    // Second sequence used same string as first one.
    puts ("4");
    puts ("5");
    return 0;
}

#ifndef PORT_HOST
int putchar (int c)
{
	static int i;
	ASSERT (c == "1\n2\n4\n5\n"[i++]);
}
#endif

void foo(void)
{
}

void
testBug(void)
{
	bug ();
}

