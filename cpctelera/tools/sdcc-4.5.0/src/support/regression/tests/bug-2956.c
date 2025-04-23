/*
   bug-2452.c
   live-range shortening would move return before assignment to global in compound op
*/

#include <testfwk.h>

#pragma disable_warning 283

unsigned int val;

unsigned int testadd()
{
    return (val += 1);
}

void testBug(void)
{
    testadd();
    ASSERT (val == 1);
}

