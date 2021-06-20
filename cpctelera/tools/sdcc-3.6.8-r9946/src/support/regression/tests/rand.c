/* rand.c
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdlib.h>

void testRand(void)
{
    /* The default seed is 1 */
	int r = rand();
	srand(1);
	ASSERT(r == rand());
}

