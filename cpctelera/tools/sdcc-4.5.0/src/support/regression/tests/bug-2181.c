/*
    bug 2181
*/

#include <testfwk.h>

#include <string.h>

#if !defined( __SDCC_mcs51) && !defined( __SDCC_pdk15) // Use shorter array for devices with low memory.
#define SROWLENGTH 36
#else
#define SROWLENGTH 6
#endif

#ifndef __SDCC_pdk14 // Not enough memory
unsigned char a[2][4][SROWLENGTH];

void f(void)
{
	unsigned char i;

	for(i = 0; i < 4; i++)
		memset(a[0][i], 144 + i * 32, SROWLENGTH);
}
#endif

void testBug(void)
{
#ifndef __SDCC_pdk14 // Not enough memory
	f();
	ASSERT (a[0][0][0] == 144 + 0 * 32);
	ASSERT (a[0][0][SROWLENGTH - 1] == 144 + 0 * 32);
	ASSERT (a[0][3][0] == 144 + 3 * 32);
	ASSERT (a[0][3][SROWLENGTH - 1] == 144 + 3 * 32);
#endif
}

