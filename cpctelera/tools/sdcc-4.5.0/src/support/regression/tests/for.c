/** Tests for loops.
 */

#include <testfwk.h>

int i23 = 0;
int i42 = 0;

void g23(int i)
{
	ASSERT(i == 23);
	i23 ^= 23;
}

void g42(int i)
{
	ASSERT(i == 42);
	i42 += 42;
}

void testFor(void)
{
	{
		int i = 23;
		g23(i);
		for(int i = 0; i < 2; i++)
		{
			int i = 42;
			g42(i);
		}
		g23(i);
	}
	ASSERT(i23 == 0);
	ASSERT(i42 == 84);
}

