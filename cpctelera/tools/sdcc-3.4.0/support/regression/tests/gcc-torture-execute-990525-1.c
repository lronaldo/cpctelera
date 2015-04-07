/*
   990525-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports struct!
#if 0
struct blah {
    int m1, m2;
};

void die(struct blah arg)
{
    int i ;
    struct blah buf[1];

    for (i = 0; i < 1 ; buf[i++] = arg)
        ;
    if (buf[0].m1 != 1) {
        ASSERT (0);
    }
}
#endif

void
testTortureExecute (void)
{
#if 0
    struct blah s = { 1, 2 };

    die(s);
    return;
#endif
}

