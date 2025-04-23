/** A bad z80 peephole optimizer rule created invalid asm
*/

// Based on code sample under GPL 2.0 or later
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <testfwk.h>

#pragma disable_warning 85

void vdu_sprite_position_set16( uint8_t id, uint8_t tile, uint8_t x, uint8_t y, uint8_t colour ) {}

///< Show patch - set tiles
uint8_t vdu_patch_crop_impl( uint8_t id, int8_t col, int8_t row,
    int8_t width, int8_t offset, const uint8_t *counts, const uint8_t *colours,
    int8_t cols, int8_t rows, uint8_t tile_offset, bool zoom ) {

    int16_t px = col;
    int16_t py = rows;

    do {

        uint8_t x = cols;
        do {

            uint8_t count = 1;

            do {
                vdu_sprite_position_set16( id++, tile_offset, px, py, *colours++ ); // The bad asm happened in code for passing parameters here.

            } while ( --count );

            px++;
            counts++;

        } while( --x );

    } while ( --rows );

    return 0;
}

void
testBug(void)
{
    uint8_t c = 0;
    vdu_patch_crop_impl(0, 0, 0, 0, 0, &c, &c, 1, 1, 0, 0);
}

