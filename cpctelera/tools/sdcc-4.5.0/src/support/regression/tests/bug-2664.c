/* bug-2664.c
   A frontend issue in the handling of functions returning function pointers (without using a typedef)
 */

#include <testfwk.h>

int i;

void (*s(int signo, void (*func)(int signo) __reentrant))(int signo)
{
	func(signo);
    return(func);
}

void (*f(int j))(void)
{
	i = j;
    return 0;
}

void g(int j) __reentrant
{
	i = j;
}

void
testBug(void)
{
	f(1);
	ASSERT (i == 1);
	s(2, &g);
	ASSERT (i == 2);
}

