/*
   bug-3230.c
   A bug in code generation for __z88dk_callee __smallc functions with parameters wider than char
 */

#include <testfwk.h>

unsigned int f2(unsigned int c) __smallc __z88dk_callee
{
	return c + 1;
}

unsigned int s2(unsigned int c, unsigned int d) __smallc __z88dk_callee
{
	return c - d;
}

void testBug(void)
{
	ASSERT (f2(2) == 3);
	ASSERT (s2(2, 3) == -1);
}

