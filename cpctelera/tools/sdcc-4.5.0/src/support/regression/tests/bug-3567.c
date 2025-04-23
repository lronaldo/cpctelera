/* bug-3255.c
   For z80, a read from a pointer overwrote register a that was in use for a local variable.
 */

#include <testfwk.h>

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later
#include <stdint.h>

#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Lack of memory
static volatile uint8_t VDPControlPort;
static volatile uint8_t VDPDataPortOut;

#define BYTE_HI(x) ((x)>>8)
#define VDU_ADDRESS_SET_MASK 0x4000

void vdu_stop() {}

#define vdu_next_set_fast(value) VDPDataPortOut = value
#define vdu_address_set_unsafe(address) { VDPControlPort=address; VDPControlPort=BYTE_HI((address) | VDU_ADDRESS_SET_MASK ); }

#define for8( var, size ) for ( unsigned char var = 0; var < size; var++ )
typedef uint8_t u8;
typedef uint16_t u16;

#define MAX_TILES 24

typedef struct {

    u16 address;
    u8 size;
    u8 tile[2];

} GameTile;

typedef struct {

    GameTile tiles[MAX_TILES];
    u8 size;

} GameTileState;

GameTileState gameTiles = { .size = 2, .tiles = { { 1, 2, { 2, 3 } }, { 6, 2, { 7, 8 } } } };

int p(int i)
{
    ASSERT (i == 2);
}

void GameTileCallback() {

    vdu_stop();

    GameTile *gameTile = gameTiles.tiles;

    for8( id, gameTiles.size ) {

        vdu_address_set_unsafe( gameTile->address );

        u8 *ptr = gameTile->tile;
        u8 size; size = gameTile->size;
        do {
            vdu_next_set_fast( *ptr++ ); // Pointer read here overwrote size, which was allocated to register a.

        } while ( --size );

        p (ptr - gameTile->tile);

        gameTile++;
    }

    gameTiles.size = 0;
}
#endif

void
testBug(void) {
#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Lack of memory
    GameTileCallback();
#endif
}

