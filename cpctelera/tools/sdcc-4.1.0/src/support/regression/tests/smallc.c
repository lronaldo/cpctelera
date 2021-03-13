/*
	smallc.c

	Test support for __smallc for Small-C-compatible calling convention
*/
#include <testfwk.h>
#include <stdlib.h>

unsigned char f1(unsigned char c) __smallc
{
	return c + 1;
}

unsigned int f2(unsigned int c) __smallc
{
	return c + 1;
}

unsigned char s1(unsigned char c, unsigned char d) __smallc
{
	return c - d;
}

unsigned int s2(unsigned int c, unsigned int d) __smallc
{
	return c - d;
}

void
testSmallC(void)
{
  ASSERT (f1 (23) == 24);
  ASSERT (f2 (23) == 24);

  ASSERT (s1 (42, 23) == 19);
  ASSERT (s2 (42, 23) == 19);
}

