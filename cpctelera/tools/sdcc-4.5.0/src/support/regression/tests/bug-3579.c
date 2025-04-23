/* bug-3579.c
   Codegen for sm83 could generate an "ex de, hl" (a Z80 instruction not supported on SM83) when passing a struct as function argument while register hl holds another variable.
   This would then result in an error message from the assembler.
 */

#include <testfwk.h>

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later
#pragma disable_warning 85

#include <stdint.h>
#include <stdbool.h>

typedef struct {

    uint8_t type;
    uint8_t state;
    uint8_t left;
    uint8_t width;
    uint8_t ingress;
    uint8_t scale;

} SpriteData;

uint8_t vdu_patch_crop_impl( uint8_t id, int8_t col, int8_t row,
    int8_t width, int8_t offset, const uint8_t *counts, const uint8_t *colours,
    int8_t cols, int8_t rows, uint16_t tile_offset, bool zoom ) {

    ASSERT( tile_offset == 8);

    return 0;
}

typedef uint8_t ImageBaseType;

typedef struct {

    const ImageBaseType * const * const * const sprites;
    uint8_t bank;

} SpriteItem;

const ImageBaseType * const I1 = {0};
const ImageBaseType * const * const I0 = &I1;
SpriteItem SI = {&I0, 0};

const SpriteItem * wolf_sheets[1] = {&SI};

uint8_t sprite_state_show( uint8_t id, SpriteData *sprite, uint8_t y, uint8_t tile ) {

    static bool zoom = false;

    // Only allow zoom on first sprite, if it's full size enemy
    if ( id == 0 ) {
        zoom = sprite->type > 1 && sprite->scale == 5;
        if ( zoom ) sprite->scale--;
    } else {
        if ( zoom ) goto finish;
    }

    const SpriteItem * sprite_data = wolf_sheets[0];

    const ImageBaseType * const * const scale_sprite_data = sprite_data[0].sprites[0];

    id = vdu_patch_crop_impl( id, sprite->left, 0, sprite->width, 0, scale_sprite_data[0], 0,0,0, tile, zoom );

 finish:

    return id;
}

void
testBug (void) {
    SpriteData sprite = { 1,2,3,4,5,6};
    sprite_state_show( 0, &sprite, 7, 8 );
}

