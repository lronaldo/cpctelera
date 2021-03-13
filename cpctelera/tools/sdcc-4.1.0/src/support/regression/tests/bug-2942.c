/*
   bug-2942.c
   An bug in lospre, resulting in overoptimistic handling of reads via pointers.
   The *p +7 was moved before the if(j) by lospre, which is wrong since p might point to i.
*/

#include <testfwk.h>

int i, j;
int *p;

void f(void)
{
	int k;

	if(j)
	{
		i++;
		k = *p + 7;
	}
	else
		k = *p + 7;
	j = k;
}

void testBug(void)
{
	p = &i;

	f();

	ASSERT (j == 7);

	f();

	ASSERT (j == 8);
}

