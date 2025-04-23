/* Fake header.
 */
#include <testfwk.h>

long leftShiftLong (long l) {
    return (l << 3);
}

int leftShiftIntMasked (int v) {
    return ((v & 0xff00U) << 3);
}

int leftShiftIntMasked2 (int v) {
    return ((v & 0xff) << 8);
}

int leftShiftIntMasked3 (int v) {
    return ((v & 0xff) << 3);
}

void testBug(void)
{
}
