/*
   bug-2254.c a bug in stm8 code generation for jumps on immediates
 */

#include <testfwk.h>

short int isArmed(void)
{
    return 1;
}

int bug(void)
{
    if (!isArmed)
        return 0;
    else
        return 1;
}

void testBug(void)
{
	ASSERT(bug());
}

