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
#ifndef __OpenBSD__ /* OpenBSD intentionally does not follow the C standard for rand() */
	ASSERT(r == rand());
#endif
}

