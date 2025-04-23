/*
   bug-3254.c. A bug in _Generic handling of implicitly assigned intrinsic named address spaces.
 */

#include <testfwk.h>

#include <string.h>

char const* a = _Generic("bla", char*: "blu"); // Failed to compile this line due to "bla" being in __code.

void testBug(void)
{
    ASSERT(!strcmp(a, "blu"));
}

