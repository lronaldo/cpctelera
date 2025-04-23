/*
   20001011-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <string.h>

int foo(const char *a)
{
    return strcmp(a, "testTortureExecute");
}

void
testTortureExecute (void)
{
    if(foo(__func__))
        ASSERT (0);
    return;
}

