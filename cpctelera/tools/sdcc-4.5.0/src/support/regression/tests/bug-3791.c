/* bug-3791.c
   Ran into an assertion in z80 code generation.
*/

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later

#include <testfwk.h>

#include <stdint.h>

#define BYTE_HI2(x) ((x)>>8)
#define vdu_address_set2(address) { VDPControlPort=address; VDPControlPort=BYTE_HI2((address) ); }
#define vdu_next_set_fast2(value) VDPDataPortOut = value
#define VDU_SPRITE_POSITION_TABLE_ADDRESS2 0x1B00

#define __IO_VDP_DATA_OUT   #0xbe
#define __IO_VDP_COMMAND    #0xbf

#ifdef __SDCC_z80
static volatile __sfr __at __IO_VDP_COMMAND VDPControlPort;
static volatile __sfr __at __IO_VDP_DATA_OUT VDPDataPortOut;
#else
static volatile unsigned char VDPControlPort;
static volatile unsigned char VDPDataPortOut;
#endif

///< Set position of sprite
void vdu_sprite_position_set_fast( uint8_t id ) {

    vdu_address_set2( VDU_SPRITE_POSITION_TABLE_ADDRESS2 + id );

    vdu_next_set_fast2( id );
}

void testBug( void )
{
}

