/* bug-3465.c
   Symbols for previous parameters were not available for the array dimension of a parameter.
 */

#include <testfwk.h>
#include <string.h>

#pragma disable_warning 85

void f(unsigned int j, char d[sizeof(j)])
{
	unsigned int k;
	memcpy (d, &j, sizeof(j));
}

void
testBug(void)
{
	unsigned int j = 0xaa55;
	char d[sizeof(j)];
	f(j, d);
	ASSERT(!memcmp(&j, d, sizeof(j)));
}

