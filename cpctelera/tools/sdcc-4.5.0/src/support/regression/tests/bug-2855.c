/*
   bug-2855.c

   Missing non-inline definition of aligned_alloc
*/

#include <testfwk.h>

#include <stdlib.h>

#if __STDC_VERSION__ >= 201112L
#if !defined(__APPLE__)
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined (__SDCC_mos6502) && !defined(__SDCC_mos65c02) || defined (__SDCC_STACK_AUTO) // Reentrancy required for function pointers.
void *(*volatile f)(size_t, size_t) = &aligned_alloc;
#endif
#endif
#endif

void testBug(void)
{
#if __STDC_VERSION__ >= 201112L
#if !defined(__APPLE__)
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined (__SDCC_mos6502) && !defined(__SDCC_mos65c02) || defined (__SDCC_STACK_AUTO)
	int *buffer = (*f)(_Alignof(int), sizeof(int) * 2);
	buffer[0] = 23;
	buffer[1] = 42;
	ASSERT (buffer[0] == 23);
	ASSERT (buffer[1] == 42);
	free (buffer);
#endif
#endif
#endif
}

