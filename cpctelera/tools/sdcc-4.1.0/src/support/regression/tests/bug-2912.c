/*
   bug-2912.c a bug in handling non-spilt register parameters in __z88dk_fastcall functions.
 */

#include <testfwk.h>

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // lack of memory
float y, z;

float m32_sinf (float x) __z88dk_fastcall
{
    y = x;
}

float m32_cosf (float x) __z88dk_fastcall
{
    z = y;
}

float m32_tanf (float x) __z88dk_fastcall
{
    return m32_sinf(x)/m32_cosf(x);
}
#endif

void testBug(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // lack of memory
    volatile float x = 23.0f;

    m32_tanf(x);

    ASSERT(y == x);
    ASSERT(z == x);
#endif
}

