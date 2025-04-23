/* bug-3780.c
   A bug in the conversion of multiplicative operators to support functions triggred an assertion.
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 88
#endif

unsigned f( char* p ) {
    return 7 * (unsigned long)p / 1234;
}

void testBug(void)
{
    ASSERT(f((char *)176) == 0);
    ASSERT(f((char *)177) == 1);
}

