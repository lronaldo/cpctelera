/*
   bug-3216.c
   A bug in r3ka code generation for 32-bit additions in handling iy
 */

#include <testfwk.h>

unsigned long c90base(void);
unsigned long c90lib(void);

unsigned long c90base_score, c90lib_score;

unsigned long stdcbench(void)
{
	unsigned long score = 0;

	score += c90base_score = c90base();

	score += c90lib_score = c90lib();

	return(score);
}

unsigned long c90base(void)
{
	return 0x01020304ul;
}

unsigned long c90lib(void)
{
	return 0x10203040ul;
}

void
testBug (void)
{
	ASSERT(stdcbench() == 0x11223344ul);
	ASSERT(c90base_score == 0x01020304ul);
	ASSERT(c90lib_score == 0x10203040ul);
}

