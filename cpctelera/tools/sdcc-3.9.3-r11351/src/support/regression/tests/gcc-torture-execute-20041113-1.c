/*
   20041113-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 93
#endif

#pragma disable_warning 85

#include <stdarg.h>

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
void test (int x, ...)
{
    va_list ap;
    int i;
    va_start (ap, x);
    if (va_arg (ap, int) != 1)
	ASSERT (0);
    if (va_arg (ap, int) != 2)
	ASSERT (0);
    if (va_arg (ap, int) != 3)
	ASSERT (0);
    if (va_arg (ap, int) != 4)
	ASSERT (0);
}

double a = 40.0;
#endif

void testTortureExecute (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
    test(0, 1, 2, 3, (int)(a / 10.0));
    return;
#endif
}
