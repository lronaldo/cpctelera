/*
   bug-2826.c
   An optimization inserted instructions in between IPUSH and CALL which resulted in wrong code for mcs51.
*/

#include <testfwk.h>

#define DS_CMD_RAM 1 << 6

unsigned char cfg_table[4];

unsigned char f(unsigned char j)
{
	static unsigned char testval = DS_CMD_RAM >> 1 | 2;
	ASSERT(j == testval);
	testval++;
	return(0);
}

void testBug(void)
{
	unsigned char i, j;
	j = DS_CMD_RAM >> 1 | 2;
	for (i = 0; i != 4; i++)
		cfg_table[i] = f(j++); // The incremented value got passed to f for mcs51.
}

