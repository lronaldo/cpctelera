/* bug-3711.c
   A bug in mcs51 code generation for __reentrant functions with __bit parameters.
*/

#include <testfwk.h>

#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390)
#define __bit _Bool
#endif

void f0(__bit b) __reentrant;
void f1(unsigned char c, __bit b) __reentrant;
void f2(__bit b, unsigned char c) __reentrant;
void f3(unsigned long long l, __bit b) __reentrant;
void f4(__bit b, unsigned long long l) __reentrant;
void f5 (unsigned char c, const unsigned char __code* cp, unsigned int i, unsigned int j, __bit b, const void __code* vcp) __reentrant;

void g(void)
{
	f0(0);
	f1(1, 0);
	f2(0, 1);
	f3(1, 0);
#ifndef __SDCC_mcs51 // Bug #3712
	f4(0, 1);
#endif
	f5(2, 0, 3, 4, 1, 0);
}

void
testBug(void)
{
	g();
}

void f0(__bit b) __reentrant
{
	ASSERT (b == 0);
}

void f1(unsigned char c, __bit b) __reentrant
{
	ASSERT (c == 1);
	ASSERT (b == 0);
}

void f2(__bit b, unsigned char c) __reentrant
{
	ASSERT (b == 0);
	ASSERT (c == 1);
}

void f3(unsigned long long l, __bit b) __reentrant
{
	ASSERT (l == 1);
	ASSERT (b == 0);
}

#ifndef __SDCC_mcs51 // Bug #3712
void f4(__bit b, unsigned long long l) __reentrant
{
	ASSERT (b == 0);
	ASSERT (l == 1);
}
#endif

void f5 (unsigned char c, const unsigned char __code* cp, unsigned int i, unsigned int j, __bit b, const void __code* vcp) __reentrant
{
	ASSERT (c == 2);
	ASSERT (cp == 0);
	ASSERT (i == 3);
	ASSERT (j == 4);
	ASSERT (b == 1);
	ASSERT (vcp == 0);
}

