/*
   bug-3172.c
   An issue in variable-argument argument promotion. Most backends, when not using --std-cXX, omit integer promotion on explicitly variable cast arguments.
   
   promotion: PROMOTE, NOPROMOTE
 */

#include <testfwk.h>

#define {promotion}

#if defined(__SDCC) && defined(PROMOTE)
#pragma std_c99
#elif defined(__SDCC) && defined(NOPROMOTE)
#pragma std_sdcc99
#endif

#include <stdarg.h>

void
receive(const char *s, ...)
{
	va_list ap;
	
	va_start(ap, s);
#if defined(__SDCC) && defined(NOPROMOTE) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
#if 0 // Bug #3172 not yet fixed.
	ASSERT(va_arg(ap, unsigned char) == 0x5a);
	ASSERT(va_arg(ap, unsigned char) == 0xa5);
#endif
#else
	ASSERT(va_arg(ap, int) == 0x5a);
	ASSERT(va_arg(ap, int) == 0xa5);
#endif
	va_end(ap);
}

void
testPass(void)
{
	receive(0, (unsigned char)0x5a, (unsigned char)0xa5);
	
	unsigned char i1 = 0x5a;
	unsigned char i2 = 0xa5;
	receive(0, (unsigned char)i1, (unsigned char)i2);
	
	unsigned char *p1 = &i1;
	unsigned char *p2 = &i2;
	receive(0, (unsigned char)(*p1), (unsigned char)(*p2));
}

