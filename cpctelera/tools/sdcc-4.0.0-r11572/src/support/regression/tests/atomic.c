/** atomic.c
*/
#include <testfwk.h>

// Some ports do not have atomic_flag yet.
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_tlcs90)

#include <stdatomic.h>
#include <stdbool.h>

atomic_flag f1 = ATOMIC_FLAG_INIT;
atomic_flag f2;

#endif

void testAtomic(void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_tlcs90)
	ASSERT(atomic_flag_test_and_set(&f1) == false);
	ASSERT(atomic_flag_test_and_set(&f1) == true);
	atomic_flag_clear(&f1);
	atomic_flag_clear(&f2);
	ASSERT(atomic_flag_test_and_set(&f1) == false);
	ASSERT(atomic_flag_test_and_set(&f2) == false);
#endif
}

