/*
    bug 1565152.c
*/

#include <testfwk.h>

// no need to call this, it generates compiler error for xstack-auto
//   error 9: FATAL Compiler Internal Error

#if defined(__SDCC_mcs51)
#include <8051.h>

unsigned int foo(void)
{
	unsigned int i,j;
	unsigned long l=0;

	for (i=0;i<10;i++)
	{
		j=SP; // any SFR would do to generate crash
		l+=j; 
	} 
	return 1; // <- it gives error on this line
}
#endif

void
testBug(void)
{
	ASSERT(1);
}
