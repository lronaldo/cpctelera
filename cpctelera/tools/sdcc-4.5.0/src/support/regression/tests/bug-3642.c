/* bug-3642.c
   A bug in live-range separation resulted in a complement operation being eliminated incorrectly.
*/

#include <testfwk.h>

volatile int sss;
long kkk;

void ss(long k)
{
	unsigned char minus;
	if(k<0)
	{
		k=~k;
		k++;
		minus=1;
	}
	else 
		minus=0;
	sss = minus;
	kkk = k+1;
}

void
testBug(void)
{
	ss(-1);
	ASSERT(sss == 1);
	ASSERT(kkk == 2);
}

