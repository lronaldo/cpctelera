/*
   bug-3256.c
   A bug in the stm8 peephole optimizer for functions calls through function pointers where the caller has no non-pointer calls, and the callee has register arguments.
 */

#include <testfwk.h>

#ifndef __SDCC_stm8
#define __raisonance
#endif

int (*volatile twocharargptr)(char, char) __raisonance __reentrant;

int f(void)
{
	return (*twocharargptr)(23, 42);
}

int twochararg(char a, char b) __raisonance __reentrant
{
	return a + b;
}

void
testBug(void)
{
	twocharargptr = &twochararg;
	ASSERT (f() == 23 + 42);
}

