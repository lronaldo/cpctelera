#include "gpsim_assert.h"

/*
 * Test for buggy pCode optimization on
 *    CLRF reg	; pc1
 *    ...
 *    MOVF reg,W	; pc2
 *
 * Originally, both instructions were removed and pc2 replaced with
 *    CLRF reg          iff reg was used afterwards, but Z and W were not, or
 *    MOVLW 0           iff reg and Z were not used afterwards, but W was.
 * Detection of W being used used to be buggy, though...
 */
signed int x=0;
unsigned char y=1;

void main() {
    x += y;
    x += y;
    if (x != 2) { FAILED(); }
    if (y != 1) { FAILED(); }
    //ASSERT(MANGLE(x) == 2);
    //ASSERT(MANGLE(y) == 1);
    PASSED();
}

