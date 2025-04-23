/*
   C declaration syntax for functions returning function pointers is controversial.
   Those who don't like it often used typedef to put the returned type in what they
   perceive as the natural location for it. With C23 typeof, the same can be achieved
   without introducing a typeof, so the idiom tested here is becoming popular.
*/

#include <testfwk.h>

#if defined(__SDCC) && defined(__SDCC_STACK_AUTO) // Don't know how to write this test without --stack-auto.
#pragma std_c23

int f(int i, int j)
{
	return i + j;
}

typeof (int (*)(int, int)) g(void)
{
	return &f; // Why do we get a warning here? Bug 3805.
}

int h(typeof (int (*)(int, int)) p, int i, int j)
{
	return (*p)(i, j);
}

int h0(int i, int j)
{
	return h(&f, i, j);
}

int h1(int i, int j)
{
	return (*g())(i, j);
}

#endif

void
testTypeofFptr(void)
{
#if defined(__SDCC) && defined(__SDCC_STACK_AUTO)
	ASSERT (f(23, 42) == 23 + 42);
	ASSERT (h0(23, 42) == 23 + 42);
	ASSERT (h1(23, 42) == 23 + 42);
#endif
}

