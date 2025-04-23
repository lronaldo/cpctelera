/*      bug-2231.c
*/

#include <testfwk.h>
#include <string.h>

char src[12] = "Hello World";
char dst[16] = "***************";

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
        strncpy(dst, src, 5);
        ASSERT(0 == memcmp(dst, "Hello**********", 16));
        strncpy(dst, src, 15);
        ASSERT(0 == memcmp(dst, "Hello World\0\0\0\0", 16));
#endif
}
