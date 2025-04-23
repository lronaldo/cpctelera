/*
    bug 2084206
*/

#include <stdarg.h>
#include <testfwk.h>

typedef struct {
	void (*f) (void);
	char c;
	char* p;
} data_t;

typedef void* voidp_t;
typedef void (*funp_t)(void);

volatile data_t mydata;

long varargs(char i, ...)
{
	va_list arg;
	long ret = -1;

	va_start (arg, i);
	switch (i)
	{
		case 1: ret = (long)va_arg(arg, voidp_t); break;
		case 2: ret = (long)va_arg(arg, funp_t);  break;
	}
	va_end (arg);
	return ret;
}

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory - see RFE # 613.
#ifndef __SDCC_pdk15 // TODO: Decide on support for casts between object and function pointers for pdk!
#ifndef __SDCC_pic16
#if !((defined __SDCC_stm8) && defined(__SDCC_MODEL_LARGE)) // STM8 large model has sizeof(void *) < size of function pointers.
#if !defined(__SDCC_ds390) && !defined(__SDCC_mcs51) // DS390 and at least some MCS-51 memory models have sizeof(void *) > size of function pointers.
	void* ptr = testBug;
	mydata.f = testBug;
	ASSERT (varargs(1, mydata.f) == (long)ptr);
	ASSERT (varargs(1, mydata.f) == (long)(void*)testBug);
	ASSERT (varargs(2, (funp_t)mydata.f) == (long)mydata.f);
	ASSERT (varargs(2, (void (*)(void))mydata.f) == (long)mydata.f);
#endif
#endif
#endif
#endif
#endif
}
