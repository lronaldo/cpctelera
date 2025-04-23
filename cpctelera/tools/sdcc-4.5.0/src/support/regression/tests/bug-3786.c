/** bug-3778.c: Some initalized variables placed in code segement instead of const segment.
*/

#include <testfwk.h>

int j;

#if defined(__SDCC_stm8) && defined(__SDCC_MODEL_LARGE) // Fill lower 32KB of flash, so f1 or f2 is above 0x10000.
#define ARRAYSIZE 32000
long k;
void dummyfunc(void)
{
	switch(k)
	{
	case 1:
		j = (j + 1) % (j - 1);
	case 2:
		j = (j - 1) % (j + 1);
	case 3:
		j = (j + 1) % (j + 1);
	case 4:
		j = (j - 1) % (j - 1);
	case 5:
		j = (j + 2) % (j - 2);
	case 6:
		j = (j - 2) % (j + 2);
	}
}
#else
#define ARRAYSIZE 1
#endif

const char c[ARRAYSIZE];

extern const unsigned int i;

void
testBug(void)
{	
	ASSERT(i == 0xa55a);
}

const unsigned int i = 0xa55a; // Would be placed in CODE instead of CONST.

