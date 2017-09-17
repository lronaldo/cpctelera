/*
   bug-2304.c a literal address bug for operands bigger than 2 bytes.
 */

#include <testfwk.h>

void testBug(void)
{
#ifdef __SDCC_pic16
	signed long *l = (signed long *) 0x02b0;
	float *f = (float *) 0x02b0;
#elif defined (__SDCC_mcs51) || defined (__SDCC_ds390)
	__xdata signed long *l = (__xdata signed long *) 0xcab0;
	__xdata float *f = (__xdata float *) 0xcab0;
#elif defined (__SDCC_stm8)
	signed long *l = (signed long *) 0x1000;
	float *f = (float *) 0x1000;
#elif defined (__SDCC)
	signed long *l = (signed long *) 0xcab0;
	float *f = (float *) 0xcab0;
#else // host test
	char buf[16];
	signed long *l = (signed long *) buf;
	float *f = (float *) buf;
#endif

	*l++ = -2;	
	*l = 2;
	ASSERT(*(l - 1) == -2);
	ASSERT(*l == 2);

	*f++ = 10;
	*f = 20;
	ASSERT(*(f - 1) == 10);
	ASSERT(*f == 20);
}

