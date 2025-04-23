/* bug-3259.c
   A bug in stm8 code generation for rematerialized addressesof stack parameters of functions returning large values (longlong, struct, etc).
 */
 
#include <testfwk.h>

long long f(long long *p);

long long g(long long ll)
{
	return f(&ll);
}

long long f(long long *p)
{
	return *p;
}

void testBug(void)
{
	ASSERT (g (0x1234567) == 0x1234567);
}

