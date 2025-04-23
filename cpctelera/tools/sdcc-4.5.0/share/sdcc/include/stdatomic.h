#ifndef __SDCC_STDATOMIC_H
#define __SDCC_STDATOMIC_H 1

#include <sdcc-lib.h>

#ifndef __SDCC_ATOMIC_EXTERN
#define __SDCC_ATOMIC_EXTERN
#endif

typedef struct {unsigned char flag;} atomic_flag;

_Bool atomic_flag_test_and_set(volatile atomic_flag *object) __SDCC_NONBANKED;

#if defined(__SDCC_mcs51) || defined(__SDCC_ds390)

#define ATOMIC_FLAG_INIT {0}

__SDCC_ATOMIC_EXTERN
inline void atomic_flag_clear(volatile atomic_flag *object) __SDCC_NONBANKED
{
	object->flag = 0;
}

#elif defined(__SDCC_f8)

#define ATOMIC_FLAG_INIT {0}

_Bool atomic_flag_test_and_set(volatile atomic_flag *object);

__SDCC_ATOMIC_EXTERN
inline void atomic_flag_clear(volatile atomic_flag *object)
{
	object->flag = 0;
}

#elif defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_ez80_z80) || defined(__SDCC_z80n) || defined(__SDCC_sm83) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka) || defined(__SDCC_r800)

#define ATOMIC_FLAG_INIT {0xfe}

__SDCC_ATOMIC_EXTERN
inline void atomic_flag_clear(volatile atomic_flag *object)
{
	object->flag = 0xfe;
}

#elif defined(__SDCC_tlcs90) || defined(__SDCC_stm8) || defined(__SDCC_hc08) || defined(__SDCC_s08) || defined(__SDCC_mos6502) || defined(__SDCC_mos65c02)

#define ATOMIC_FLAG_INIT {1}

__SDCC_ATOMIC_EXTERN
inline void atomic_flag_clear(volatile atomic_flag *object)
{
	object->flag = 1;
}

#else

#error Support for atomic_flag not implemented

#endif

#endif

