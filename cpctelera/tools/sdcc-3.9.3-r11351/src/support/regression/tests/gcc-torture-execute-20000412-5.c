/*
   20000412-5.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

void
testTortureExecute (void) {
    struct {
	int node;
	int type;
    } lastglob[1] = { { 0   , 1  } };

    if (lastglob[0].node != 0 || lastglob[0].type != 1)
      ASSERT (0);
    return;
}

