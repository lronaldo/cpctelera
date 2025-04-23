/*
    bug 3366711
*/

#include <testfwk.h>

void f(unsigned char c)
{
	unsigned char __xdata * p = (unsigned char __xdata *)8;
	p[3] = c ? 2 : 1; /* Old register allocator crashed in this assignment to a constant address. */
}

void
testBug (void)
{
  ASSERT (1);
}

