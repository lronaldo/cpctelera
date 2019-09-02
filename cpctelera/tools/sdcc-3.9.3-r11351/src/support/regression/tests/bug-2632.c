/*
   bug-2632.c

   A temporary in an optimization for address calculations for array accesses used a signed 8-bit type to hold values up to 255, and then cast the result to a signed int.
*/

#include <testfwk.h>

#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Not enough memory
unsigned char testArr255[255];  // Fails
unsigned char testArr256[256];  // Succeeds

void g(char *p)
{
	*p = 0;
}

void f(void)
{
    int i, j;

    i = 5;
    for (j = 7; j < 10; j++)
    {
        g(&testArr255[i * 24 + j]);
        g(&testArr256[i * 24 + j]);
    }
}
#endif

void testBug(void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Not enough memory
	for(unsigned int i = 0; i < 250; i++)
		testArr255[i] = i;
	for(unsigned int i = 0; i < 250; i++)
		testArr256[i] = i + 1;

	f();

	for(unsigned int i = 0; i < 127; i++)
		ASSERT(testArr255[i] == i);
	for(unsigned int i = 0; i < 127; i++)
		ASSERT(testArr256[i] == i + 1);
	for(unsigned int i = 127; i < 130; i++)
		ASSERT(testArr255[i] == 0);
	for(unsigned int i = 127; i < 130; i++)
		ASSERT(testArr256[i] == 0);
	for(unsigned int i = 130; i < 250; i++)
		ASSERT(testArr255[i] == i);
	for(unsigned int i = 130; i < 250; i++)
		ASSERT(testArr256[i] == i + 1);
#endif
}

