/** bug-3802.c: False positive type conversion error involving nullptr
 */

#include <testfwk.h>

#ifdef __SDCC
_Pragma("std_c23")
#endif

void testBug(void)
{
#ifdef __SDCC
    const char *nullCharPtr = nullptr;
    ASSERT(nullCharPtr == nullptr);
#endif
}
