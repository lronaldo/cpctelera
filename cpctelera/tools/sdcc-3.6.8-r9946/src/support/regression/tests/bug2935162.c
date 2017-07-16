/*
   bug2935162.c
*/

#pragma disable_warning 184

#include <testfwk.h>
#include <limits.h>

#if defined(__STDC_VERSION__)
# if (__STDC_VERSION__ >= 199901L)
#  define IS_C99
# endif
#endif

const float a[] =
{
    LONG_MAX,       /* .byte #0x00,#0x00,#0x00,#0x4F ; +2.147484e+09   */
    LONG_MIN,       /* .byte #0x00,#0x00,#0x00,#0x4F ; +2.147484e+09 ? */

    12345678901,    /* .byte #0x00,#0x00,#0x80,#0x4F ; +4.294967e+09   */
    -12345678901,   /* .byte #0x00,#0x00,#0x80,#0x3F ; +1.000000e+00 ? */

    12345678901L,   /* .byte #0x00,#0x00,#0x00,#0x4F ; +2.147484e+09   */
    -12345678901L,  /* .byte #0x00,#0x00,#0x00,#0xCF ; -2.147484e+09   */

    2147483647L,    /* .byte #0x00,#0x00,#0x00,#0x4F ; +2.147484e+09   */
#ifdef IS_C99
    -2147483648L,   /* .byte #0x00,#0x00,#0x00,#0xCF ; -2.147484e+09   */
#else
    -2147483647L-1, /* .byte #0x00,#0x00,#0x00,#0xCF ; -2.147484e+09   */
#endif
    ULONG_MAX,      /* .byte #0x00,#0x00,#0x80,#0x4F ; +4.294967e+09   */
    1.0,            /* .byte #0x00,#0x00,#0x80,#0x3F ; +1.000000e+00   */

    0.0,            /* .byte #0x00,#0x00,#0x00,#0x00 ; +0.000000e+00   */
    -0.0            /* .byte #0x00,#0x00,#0x00,#0x80 ; -0.000000e+00   */
};

void testBug (void)
{
#ifndef __SDCC_pic16
    volatile int right;

    right = -120;

/* These first two tests assume LONG_MAX and LONG_MIN are 32-bit. This */
/* is not true for some 64-bit host compilers. Check SDCC only. */
#ifdef __SDCC
    ASSERT(a[0] > +2.1e9 && a[0] < +2.2e9);
    ASSERT(a[1] < -2.1e9 && a[1] > -2.2e9);
#endif
    ASSERT(a[2] > +2.1e9);
    ASSERT(a[3] < -2.1e9);
    ASSERT(a[4] > +2.1e9);
    ASSERT(a[5] < -2.1e9);
    ASSERT(a[6] > +2.1e9);
    ASSERT(a[7] < -2.1e9);
    ASSERT(a[8] > +4.2e9);
#endif
}

