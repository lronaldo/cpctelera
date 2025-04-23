/** z88dk.c
*/
#include <testfwk.h>
#include <stdlib.h>

unsigned char f1(unsigned char c) __z88dk_callee
{
	return c + 1;
}

unsigned int f2(unsigned int c) __z88dk_callee
{
	return c + 1;
}

unsigned int f3(unsigned char c, unsigned char d) __z88dk_callee __smallc __reentrant
{
	return c + d;
}

#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02)
unsigned long int f4(unsigned long int c) __z88dk_callee
#else
unsigned long int f4(unsigned long int c) __z88dk_callee __reentrant
#endif
{
	return c + 1;
}

unsigned char (*p1)(unsigned char) __z88dk_callee;
unsigned int (*p2)(unsigned int) __z88dk_callee;
unsigned int (*p3)(unsigned char, unsigned char) __z88dk_callee __smallc __reentrant;
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02)
unsigned long int (*p4)(unsigned long int) __z88dk_callee;
#else
unsigned long int (*p4)(unsigned long int) __z88dk_callee __reentrant;
#endif

int j;

void g0(void)
{
	j++;
}

void g1(int i) __z88dk_callee
{
	j = i;
	g0(); // Tail call optimization needs to do extra work here, to avoid skipping stack cleanup.
}

void
testZ88dk(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
  ASSERT (f1 (23) == 24);
  ASSERT (f2 (23) == 24);
  ASSERT (f3 (23, 42) == 65);
  ASSERT (f4 (23) == 24);

  p1 = &f1;
  p2 = &f2;
  p3 = &f3;
  p4 = &f4;

  ASSERT ((*p1) (23) == 24);
  ASSERT ((*p2) (23) == 24);
  ASSERT ((*p3) (23, 42) == 65);
  ASSERT ((*p4) (23) == 24);
#endif

  g1(1);
  ASSERT (j == 2);
}

