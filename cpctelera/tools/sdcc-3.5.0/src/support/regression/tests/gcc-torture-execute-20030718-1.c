/*
   20030718-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR c/10320
   The function temp was not being emitted in a prerelease of 3.4 20030406. 
   Contributed by pinskia@physics.uc.edu */

static inline void temp();
void
testTortureExecute (void)
{
        temp();
        return;
}
static void temp(){}

