/* bug-3601.c
   A bug in the font-end optimization for divisions by powers of 2.
   Basically the check for the divisior being a power of two cast it to unsigned long first, resulting in false positives.
*/

#include <testfwk.h>

#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) // Lack of memory.
unsigned long long f0(unsigned long long i)
{
	return (i / ((1ull << 16) | (1ull << 48)));
}

long long f1(long long i)
{
	return (i / -0x080000000ll);
}
#endif

void testBug(void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) // Lack of memory.
	ASSERT (f0((1ull << 16) | (1ull << 48)) == 1);
	ASSERT (f1(0x080000000ll) == -1);
#endif
}

