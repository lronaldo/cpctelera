/*
   bug-2643.c
 */

#include <testfwk.h>

struct S { int a; union { int b; int c; }; };

struct S x = { 1, { 2 } };
struct S y = { 5, { 6 } };
struct S z = { 7, { .c = 8 } };

void testBug(void)
{
	ASSERT (x.a == 1);
	ASSERT (x.b == 2);
	ASSERT (y.a == 5);
	ASSERT (y.b == 6);
	ASSERT (z.a == 7);
	ASSERT (z.c == 8);
}

