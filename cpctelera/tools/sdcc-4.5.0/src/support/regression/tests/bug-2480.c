/*
   bug-2480.c
 */

#include <testfwk.h>

typedef unsigned char uint8_t;

uint8_t v[3] = {0xaa, 0xaa, 0xaa};

void testBug (void)
{
    uint8_t aa;
    uint8_t bb;

    aa = v[0];
    bb = aa & 0x80;
    aa &= 0x7F;
    v[1] = bb;
    v[2] = aa;

    ASSERT (v[0] == 0xaa);
    ASSERT (v[1] == 0x80);
    ASSERT (v[2] == 0x2a);
}
