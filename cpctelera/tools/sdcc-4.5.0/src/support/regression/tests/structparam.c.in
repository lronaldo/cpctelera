/*  Test struct / union parameters

    type: char, int, long, long long

 */

#include <testfwk.h>

struct s
{
	{type} a;
	{type} b;
};

// Callee
{type} f(struct s s)
{
	return s.a + s.b;
}

// Caller
#ifndef __SDCC_pdk14 // Lack of memory
{type} g({type} i, {type} j)
{
	struct s s = {i, j};
	return f(s);
}
#endif

void testParam (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
	ASSERT (g(23, 42) == 23 + 42);
#endif
}

