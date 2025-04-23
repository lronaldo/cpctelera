/* bug-3276.c
 * A billion became zero when converted to and from float.  Also test related range issues.
 */

#include <testfwk.h>

volatile unsigned long ul1;
volatile unsigned long ul2;
volatile float f1;

void
testBug(void)
{
#if !defined(__SDCC_pdk14)
    ul1 = 1000000000ul;
    f1 = (float)ul1;
    ASSERT (f1 == 1000000000.0f);
    ul2 = (unsigned long)f1;
    ASSERT (ul2 == ul1);
    ul1 = 4000000000ul;
    f1 = (float)ul1;
    ASSERT (f1 == 4000000000.0f);
    ul2 = (unsigned long)f1;
    ASSERT (ul2 == ul1);
    ul1 = 0xfffffffful;
    f1 = (float)ul1;
    ASSERT (f1 == 4294967296.0f);
#endif
}
