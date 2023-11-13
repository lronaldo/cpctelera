/*
   bug-2881.c
   stack pointer adjustment code overwrote register parameter.
 */

#include <testfwk.h>

void g(void *p)
{
    p;
}

int f(int test) __z88dk_fastcall
{
    char buffer[10];
    g(buffer);
    return test + test;
}

void
testBug(void)
{
    ASSERT(f(21) == 42);
}

