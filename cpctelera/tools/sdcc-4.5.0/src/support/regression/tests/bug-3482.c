/* bug-3685.c
   A non-conncted live range resulted in wrong code generation at a (potential) tail call.
*/

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later

#include <testfwk.h>

#pragma disable_warning 85

#include <stdint.h>

uint16_t fast_rand( void );

void StatePlayerHit( uint8_t amount );

void PlayerHit( int8_t percent ) {

    percent = fast_rand() % 8 + 8;

    StatePlayerHit( percent ); // Code that corrupts the stack was generated for this call.
}

void
testBug(void)
{
	PlayerHit( 0 );
}

uint16_t fast_rand( void ) {
	return 0;
}

void StatePlayerHit( uint8_t amount ) {
}

