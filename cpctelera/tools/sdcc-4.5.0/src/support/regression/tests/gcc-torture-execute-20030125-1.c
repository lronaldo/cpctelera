/*
   20030125-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports double!
#if 0
/* Verify whether math functions are simplified.  */
#include <math.h>

float 
t(float a)
{
	return sin(a);
}
float 
q(float a)
{
	return floor(a);
}
double
q1(float a)
{
	return floor(a);
}
#endif

void
testTortureExecute (void)
{
#if 0
	if (t(0)!=0)
		ASSERT (0);
	if (q(0)!=0)
		ASSERT (0);
	if (q1(0)!=0)
		ASSERT (0);
	return;
#endif
}


