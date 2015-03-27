/*
   bug-2205.c
*/

#include <testfwk.h>

#if !defined(PORT_HOST)
#  pragma disable_warning 59
#endif

static char m = 2;

void *two__init (void)
{
    if(m)
        return;
    m = 1;
}

void testBug(void)
{
    two__init();
    ASSERT(m == 2);
}
