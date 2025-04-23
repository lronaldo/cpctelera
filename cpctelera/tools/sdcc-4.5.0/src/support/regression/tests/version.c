/* Ensure version macros are present
 */

#include <testfwk.h>

#ifdef __SDCC
unsigned int major = __SDCC_VERSION_MAJOR;
unsigned int minor = __SDCC_VERSION_MINOR;
unsigned int patch = __SDCC_VERSION_PATCH;
unsigned int revision = __SDCC_REVISION;
#endif

void testVersion(void)
{
}

