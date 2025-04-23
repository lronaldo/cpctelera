/* bug-3478.c
   A peephole optimizer issue destroyed a jump table.
 */

#include <testfwk.h>

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {

    blockWoodLeft,
    blockWood,
    blockWoodRight,
    blockWoodUnderLeft,
    blockWoodOverLeft,
    blockWoodUnderRight,
    blockWoodOverRight,
    blockEscalatorLeft,
    blockEscalatorMoveLeft,
    blockEscalatorMoveRight,
    blockEscalatorRight,
    blockJumpLeft,
    blockJumpRight,

} Blocks;

bool IsFloor( uint8_t tile ) {

    switch( tile ) {

        case blockWood:
        case blockWoodRight:
        case blockWoodUnderLeft:
        case blockWoodOverLeft:
        case blockWoodUnderRight:
        case blockWoodOverRight:
        case blockEscalatorLeft:
        case blockEscalatorMoveLeft:
        case blockEscalatorMoveRight:
        case blockEscalatorRight:
        case blockJumpLeft:
        case blockJumpRight:
            return true;
    }

    return false;
}

void
testBug(void) {
    ASSERT( IsFloor(blockJumpRight) == true );
    ASSERT( IsFloor(blockJumpLeft) == true );
}

