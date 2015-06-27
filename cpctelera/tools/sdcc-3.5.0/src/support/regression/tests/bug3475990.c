/* Due to the specific structure of the control-flow graph there was a segmentation fault in iCode generatuion. */

#include <testfwk.h>

void set_b(void)
{
}

#if defined(__SDCC)
__addressmod set_b b;
#else
#define b
#endif

int a0, a1, a2;
b int b0;
int b2, b1;
int x0, x1;

int f(void)
{
	switch(x0)
	{
	case 0:
		if(a0 && b0)
		{
		}
		else
		{
			x0++;
		}
		return(b2);
	default:
		return(x1);
	}
	return(a2);
}

void testBug(void)
{
}

