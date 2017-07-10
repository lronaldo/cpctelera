/*
   bug2938110.c
 */

#include <testfwk.h>

// no need to call this, it generates compiler error:
//   Internal error: validateOpType failed in
//   OP_SYMBOL(IC_LEFT(ic)) @ src/ds390/ralloc.c:2660:
//   expected symbol, got value
int
foo (int b)
{
	int a = 10;
	a += b;
	return a;
}

void
testBug (void)
{
	ASSERT (1);
}
