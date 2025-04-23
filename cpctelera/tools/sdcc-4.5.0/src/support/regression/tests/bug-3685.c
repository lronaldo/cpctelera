/* bug-3685.c
   Wrong code was generated for an addition involving an operand using exactly one byte of iy.
*/

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later

#include <testfwk.h>

#pragma disable_warning 85

#include <stdint.h>
#include <stdbool.h>

typedef struct {

    uint8_t page;

} VduTileData;

VduTileData vduTileData;

inline uint16_t vdu_tile_page_address( uint8_t page ) {

    return page == 1 ? 0x9000 : 0x9800;
}

inline uint16_t vdu_tile_page_address_get_fast( void ) {

    return vdu_tile_page_address( vduTileData.page );
}

void vdu_tiles_put( uint16_t offset, const uint8_t *pixels, const uint8_t *colours ) {

	static uint16_t o;
	ASSERT (offset == o);
	o += 256;
}

uint16_t decompress_vdu(const uint8_t *compressed_data, uint16_t dest ) {}

///< Render image to screen
void vdu_image_put( const uint8_t **pixels, const uint8_t **colours, const uint8_t *index ) {

    ///< Load data
    uint16_t segmentOffset = 0;
    for( uint8_t segment = 0; segment < 3; segment++ ) {

        vdu_tiles_put( segmentOffset, pixels[segment], colours[segment] );
        segmentOffset += 256; // 256
    }

    // Write index table
    if ( index ) {

        decompress_vdu( index, vdu_tile_page_address_get_fast() );
    }
}

void
testBug(void)
{

    vduTileData.page = 0;

    const uint8_t *const pixels[] = { 0, 0, 0 };
    const uint8_t *const colours[] = { 0, 0, 0 };
    const uint8_t index[] = { 0 };

    vdu_image_put( pixels, colours, index );
}

extern uint16_t vdu_tile_page_address( uint8_t page );

extern uint16_t vdu_tile_page_address_get_fast( void );
