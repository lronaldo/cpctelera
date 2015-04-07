/*
    bug 1788177
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>

#ifdef __bool_true_false_are_defined

bool var;

// no need to call this, it generates compiler error:
//   Caught signal 11: SIGSEGV
void foo(bool parm) {
	var = parm;
}

#endif

void
testBug(void)
{
	ASSERT(1);
}
