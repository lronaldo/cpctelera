/*
    bug 3299577
*/

#include <testfwk.h>

void DoProc (signed char col, signed char row, unsigned char spr)
{
	col;
	spr;
	ASSERT(row != 1);
}

unsigned char y;

void TheBug (void)
{
	unsigned char i,j,n,spr=0;
	for (j=0; j<=1; j++) {
		for (n=1; n<=7; n++) {
			for (i=2; i<=7; i++) {
				DoProc(30, i*y, spr); // Here i*y == 1 every time
			}
		}
	}
}

void
testBug(void)
{
	y = 1;
	TheBug();
}
