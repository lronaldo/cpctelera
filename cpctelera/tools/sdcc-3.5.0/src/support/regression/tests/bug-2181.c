/*
    bug 2181
*/

#include <testfwk.h>

#include <string.h>

#ifndef __SDCC_mcs51
#define SROWLENGTH 36
#else
#define SROWLENGTH 6
#endif

unsigned char a[2][4][SROWLENGTH];

void f(void)
{
	unsigned char i;

	for(i = 0; i < 4; i++)
		memset(a[0][i], 144 + i * 32, SROWLENGTH);
}

void testBug(void)
{
	f();
	ASSERT (a[0][0][0] == 144 + 0 * 32);
	ASSERT (a[0][0][SROWLENGTH - 1] == 144 + 0 * 32);
	ASSERT (a[0][3][0] == 144 + 3 * 32);
	ASSERT (a[0][3][SROWLENGTH - 1] == 144 + 3 * 32);
}

