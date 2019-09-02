/*
vrp-2.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

int f (int a) {
	if (a != 2) {
		a = a > 0 ? a : -a;
		if (a == 2)
		  return 0;
		return 1;
	}
	return 1;
}

void
testTortureExecute (void) {
	if (f (-2))
		ASSERT (0);
	return;
}

