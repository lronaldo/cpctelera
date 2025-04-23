/* bug-3459.c
   An issue that resulted in a value in register hl being overwritten on z80.
 */

#include <testfwk.h>

/// Test derived from code by SF user "Under4Mhz" in 2022.
/// GPL 2.0 or greater
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PICTURE_TILE_COUNT 4
#define PICTURE1_TILES 34
#define PICTURE2_TILES ( PICTURE1_TILES + PICTURE_TILE_COUNT )

///< Is matching tiles
bool IsSameTile( int8_t tile1, int8_t tile2 ) {

    if ( tile1 <= 0 || tile2 <= 0 ) return false;

    bool same = tile1 == tile2;
    bool flowers = tile1 > PICTURE1_TILES && tile1 <= PICTURE2_TILES && tile2 > PICTURE1_TILES && tile2 <= PICTURE2_TILES;
    bool seasons = tile1 > PICTURE2_TILES && tile2 > PICTURE2_TILES;

    return same || seasons || flowers;
}

void
testBug( void ) {
    ASSERT ( IsSameTile(PICTURE1_TILES + 1, PICTURE1_TILES + 2) );
    ASSERT ( IsSameTile(PICTURE2_TILES + 2, PICTURE2_TILES + 3) );
    ASSERT ( IsSameTile(PICTURE1_TILES + 2, PICTURE1_TILES + 3) );
    ASSERT ( IsSameTile(PICTURE2_TILES + 1, PICTURE2_TILES + 2) );
}

