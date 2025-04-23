/* bug-3503.c
   A z80 (and pdk with --stack-auto) codegen issue when comparing pointers to the stack.
 */

#include <testfwk.h>

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later
#include <stdint.h>

#pragma disable_warning 283

typedef struct {

    uint16_t object;

} SegmentData;

void RayDraw() {

    SegmentData segments[22];
    SegmentData *segment = segments;
    uint8_t pos = 0;

    do {

        if ( segment >= segments + 1 ) // Assertion in codegen triggered.
            segment--;

    } while( pos++ < 32 );
}

void
testBug(void)
{
	RayDraw();
}

