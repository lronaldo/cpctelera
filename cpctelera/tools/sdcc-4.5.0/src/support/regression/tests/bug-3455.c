/* bug-3523.c
   An issue in live-range analysis that resulted in a segfault when the first basic block in the successor list was empty.
 */

#include <testfwk.h>

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later
#include <stdlib.h>

void t() {
    //calculate width of the 
    int w = abs(8);
    int var;
    for(int i = 0; i < 10; i++)
    {
        var = 256 * (-w / 2) / 256;
    }
}

void
testBug (void)
{
	t ();
}

