/* bug-3593.c
   A bug in a mos6502 peephole rule.
 */

#include <testfwk.h>

#pragma disable_warning 85

static unsigned char bug(unsigned char a, unsigned char dummy) // The dummy parameter is set in the caller using lda, which changes the flags after setiing a.
{
        unsigned char i = 0;

        if (a == 0) { // Peephole rule eliminated a load here, desppite this changing the flags still needed for a subsequent conditional branch.
                return 0;
        }

        while (i != a) {
                ++i;
        }

        return 1;
}

void
testBug (void)
{
	ASSERT (bug (0, 1) == 0);
	ASSERT (bug (1, 0) == 1);
}

