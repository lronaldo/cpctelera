/* bug-3474.c
   strtoul failed to report the error for some overflow corner cases.
 */

#include <testfwk.h>

#include <stdlib.h>
#include <limits.h>
#include <errno.h>

void
testBug (void)
{
#if ULONG_MAX <= 0xffffffff
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) // Lack of memory. TODO: Enable once strtoul uses a more efficient implementation.
	char s[] = "5000000000";
	unsigned long l = strtoul(s, NULL, 10);
	ASSERT(l == ULONG_MAX);
	ASSERT(errno == ERANGE);
#endif
#endif
}

