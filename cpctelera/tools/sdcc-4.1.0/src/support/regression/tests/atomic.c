/** atomic.c
*/
#include <testfwk.h>

// Some ports do not have atomic_flag yet.
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)

#include <stdatomic.h>
#include <stdbool.h>

#if (defined(__SDCC_mcs51) && !defined(__SDCC_MODEL_SMALL)) || defined(__SDCC_ds390)
#define memory __idata	/* mcs51 atomic_flag must be in __data or __idata */
#else
#define memory
#endif

memory atomic_flag f1 = ATOMIC_FLAG_INIT;
memory atomic_flag f2;
memory struct { int a; atomic_flag f; } s = {0, ATOMIC_FLAG_INIT};

#endif

void testAtomic(void)
{
#if !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
	ASSERT(atomic_flag_test_and_set(&f1) == false);
	ASSERT(atomic_flag_test_and_set(&f1) == true);
	atomic_flag_clear(&f1);
	atomic_flag_clear(&f2);
	ASSERT(atomic_flag_test_and_set(&f1) == false);
	ASSERT(atomic_flag_test_and_set(&f2) == false);

	ASSERT(atomic_flag_test_and_set(&s.f) == false);
	ASSERT(atomic_flag_test_and_set(&s.f) == true);
	atomic_flag_clear(&s.f);
	ASSERT(atomic_flag_test_and_set(&s.f) == false);
#endif
}

