/* bug-2834.c
   A bug in union initialization.
 */

#include <testfwk.h>

union u
{
	int i;
	int j;
};

_Bool check(union u *u)
{
	return (u->i == 23);
}

void testBug1(void)
{
	union u u = {23};
	ASSERT (check(&u));
}

void testBug2(void)
{
	union u u = {.i = 23};
	ASSERT (check(&u));
}

void testBug3(void)
{
	union u u = {.j = 23};
	ASSERT (check(&u));
}

