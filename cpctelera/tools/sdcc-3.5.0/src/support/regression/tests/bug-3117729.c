/*
	Bug 3117729
 	For
 	x = x + state[i] + pKey[mod_16x8(i, iKeyLen)];
 	an invalid
 	add	a,(#_x + 0)
 	was generated.
 */


#include <testfwk.h>

#define UCHAR	unsigned char
#define USHORT	unsigned short

#define PCHAR	unsigned char *

#pragma disable_warning 85

UCHAR mod_16x8(USHORT s16, UCHAR i8)
{
	return (0);
}

__xdata UCHAR state[256];
UCHAR x, y;

void swap(PCHAR pa, PCHAR pb)
{
}

void RC4Init(PCHAR pKey, UCHAR iKeyLen)
{
	UCHAR i;

	i = 0;
	do
	{
		state[i] = i;
		i ++;
	} while (i);

	x = 0;
	do
	{
		x = x + state[i] + pKey[mod_16x8(i, iKeyLen)];
		swap(&state[i], &state[x]);
		i ++;
	} while(i);
	x = 0;
	y = 0;
}

void
testBug(void)
{
	ASSERT(1);
}
