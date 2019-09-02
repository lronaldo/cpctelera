/*
   bug3564649.c
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85

void g(void *p)
{
}

_Noreturn void e(void)
{
  while (1) {}
}

void f(int i)
{
	void *p;

	if(i == 7)
		p = 0;
	else
		e();

	g(p); /* False "may be used before initialization" warning occoured here. */
}
#endif

void testBug(void)
{
}

