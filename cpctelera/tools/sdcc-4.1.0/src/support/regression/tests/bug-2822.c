/* bug-2822.c
   Overwritten register in call to __z88dk_fastcall function pointer in struct.
 */

#include <testfwk.h>

#include <stdint.h>

#ifndef __SDCC_pdk14 // Lack of memory
struct wibble
{
    int (*wobble)(uint8_t *x) __z88dk_fastcall __reentrant;
} w;

struct wibble *wubble = &w;
uint8_t xp[4];

void foo(uint8_t minor)
{
    uint8_t x;
    uint8_t err;
    uint8_t *bar = xp + minor;
    
    for(x = 0; x < 4; x++) {
        err = wubble->wobble(bar);
        if (err)
            wubble->wobble(bar);
    }
}

uint8_t called;

int f(uint8_t *x) __z88dk_fastcall __reentrant
{
    ASSERT(*x == 0x5a);
    called++;
    return(1);
}

struct wibble2
{
    int (*wobble2)(uint32_t) __z88dk_fastcall __reentrant;
} w2;

struct wibble2 *wubble2 = &w2;
uint32_t xp2[4];

void foo2(uint8_t minor)
{
    uint8_t x;
    uint8_t err;
    uint32_t *bar = xp2 + minor;
    
    for(x = 0; x < 4; x++) {
        err = wubble2->wobble2(*bar);
        if (err)
            wubble2->wobble2(*bar);
    }
}

uint8_t called2;

int f2(uint32_t x) __z88dk_fastcall __reentrant
{
    ASSERT(x == 0x1155aa88);
    called2++;
    return(1);
}
#endif

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
    wubble->wobble = &f;
    xp[1] = 0x5a;
    foo(1);
    ASSERT(called == 8);

    wubble2->wobble2 = &f2;
    xp2[1] = 0x1155aa88;
    foo2(1);
    ASSERT(called2 == 8);
#endif
}

