/*
   pr33142.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdlib.h>

int lisp_atan2(long dy, long dx) {
    if (dx <= 0)
        if (dy > 0)
            return abs(dx) <= abs(dy);
    return 0;
}

void
testTortureExecute (void)
{   
    volatile long dy = 63, dx = -77;
    if (lisp_atan2(dy, dx))
        ASSERT(0);
    return;
}

