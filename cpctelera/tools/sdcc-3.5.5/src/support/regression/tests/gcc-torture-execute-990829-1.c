/*
   990829-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

double test (const double le, const double ri)
{
	double val = ( ri - le ) / ( ri * ( le + 1.0 ) );
	return val;
}

void
testTortureExecute (void)
{
        double retval;

	retval = test(1.0,2.0);
        if (retval < 0.24 || retval > 0.26)
	  ASSERT (0);
	return;
}

