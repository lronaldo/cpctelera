/* bug-2716.c
 */

#include <testfwk.h>

struct s
{
	unsigned int i;
};

struct s *p;

int i, j;

int g(void)
{
	ASSERT(0);

	return(0);
}

int f(void)
{
	int a = i;
	int b = j;

	if(p->i & 4) /* This bit test used the wrong byte of p->i*/
		g();

	return(a + b);
}

void testBug(void)
{
	struct s s;
	s.i = 0xff00;
	p = &s;
	f();
}

