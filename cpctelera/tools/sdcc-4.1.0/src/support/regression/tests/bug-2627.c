/*
    bug-2627.c, an ifxForOp issue
*/

#include <testfwk.h>

int i, j, k;

void f(void)
{
	goto l2;

l1:
	if(i < j)
		i++;

l2:
	if(i < j)
		goto l1;
	j++;
}

void g(void)
{
	goto l2;

l1:
	k++;

	if(i < j)
		i++;

l2:
	if(i < j)
		goto l1;
}

void
testBug(void)
{
	i = 0;
	j = 1;
	k = 0;

	f();
	ASSERT(i == 1);
	ASSERT(j == 2);
	ASSERT(k == 0);

	g();
	ASSERT(i == 2);
	ASSERT(j == 2);
	ASSERT(k == 1);
}
