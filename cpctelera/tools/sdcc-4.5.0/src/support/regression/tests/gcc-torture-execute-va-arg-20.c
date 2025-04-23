/*
va-arg-20.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#pragma disable_warning 85

#include <stdarg.h>

void foo(va_list v)
{
    unsigned long long x = va_arg (v, unsigned long long);
    if (x != 16LL)
	ASSERT(0);
}

void bar(char c, char d, ...)
{
    va_list v;
    va_start(v, d);
    foo(v);
    va_end(v);
}

void
testTortureExecute (void)
{
    bar(0, 0, 16LL);
    return;
}
