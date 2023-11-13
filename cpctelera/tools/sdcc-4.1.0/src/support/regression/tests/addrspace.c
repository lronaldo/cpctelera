/** addrspace.c
*/

#include <testfwk.h>

void set_a(void)
{
}

void set_b(void)
{
}

#if !defined(PORT_HOST)
__addressmod set_a space_a;
__addressmod set_b const space_b;
#endif

/* We don't really test for named address spaces working here,
   since that would require support in the simulators, and would
   make the test target-specific.
   But we can test that things that should compile compile, and that
   the named address spaces don't break other things.
*/

void testSpace(void)
{
}

