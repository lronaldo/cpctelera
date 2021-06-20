/*
   bug-?.c
*/

#include <testfwk.h>

#include <stdarg.h>

int i;

static inline void f(int x, ...)
{
	va_list v;

	va_start(v, x);

	i = va_arg(v, int);

	va_end(v);
}

void testBug(void)
{
	f(0, 1);

	ASSERT(i == 1);
}

