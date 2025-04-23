/* bug-3547.c
   A bug in the promotion of char operands of the ternary operator.
 */

#include <testfwk.h>

int f(unsigned char u) {
    return u&4 ? u : -1; // returns negative int
    //return u&4 ? (int)u : -1; // works
}

void
testBug (void) {
    ASSERT (f(0x9f) == 0x9f);
}

