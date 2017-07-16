/*
   bug3130806.c
 */

#include <testfwk.h>

char foo[2];

void blah(char *m0, void *m1)
{
    *m0 = 1;
    m1;
}

void dostuff(unsigned char* buffer, unsigned char size)
{
	unsigned char x = 0;
	unsigned char y = 0;

    while (x < 1)
	{
		y = (size - x);

		if (y == 2)
			return;

        blah(&foo[x], &buffer[y]);

    	x += y;
    }
}

void testBug(void)
{
    unsigned char buffer[2];
    dostuff(buffer, 2);
    ASSERT(foo[0] == 0);
}
