/*
   bug-3322.c
   A bug in __naked __sdcccall(N) functions (as opposed to __sdcccall(0) __naked, which worked)
   */

#include <testfwk.h>

#pragma disable_warning 85

#if defined(__SDCC_stm8) || defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_z80n) || defined(__SDCC_sm83)
#define NAKED __naked
#define SDCCCALL0 __sdcccall(0)
#else
#define NAKED
#define SDCCCALL0
#endif

static unsigned char test_2(const unsigned char foo) NAKED SDCCCALL0  {
#if defined(__SDCC_stm8) && defined(__SDCC_MODEL_MEDIUM)
    __asm
        ld   a, (0x03, sp)
        inc  a
        ret
    __endasm;
#elif defined(__SDCC_stm8) && defined(__SDCC_MODEL_LARGE)
    __asm
        ld   a, (0x04, sp)
        inc  a
        retf
    __endasm;
#elif defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_z80n)
    __asm
        ld   iy, #2
        add  iy, sp
        ld   l, 0 (iy)
        inc  l
        ret
    __endasm;
#elif defined(__SDCC_sm83)
    __asm
        ldhl sp,	#2
        ld   e, (hl)
        inc  e
        ret
    __endasm;
#else
    return foo + 1;
#endif
}

void testBug(void) {
    volatile unsigned char bar = test_2(23);
    ASSERT(bar == 24);
}

