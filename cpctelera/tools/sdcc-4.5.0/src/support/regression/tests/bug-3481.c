/* bug-3481.c
   A crash in iCode generation for parameters to calls to functions without prototypes.
 */

#include <testfwk.h>

// Function declarations without prototypes no longer exist in C2X.
#pragma std_c99
#pragma disable_warning 283

// Test derived from code by SF user "Under4Mhz" in 2022.

#include <stdint.h>

#if 0 // Bug not yet fixed
uint16_t fast_rand();

void ImageStatusImage( uint8_t health ) {
    uint8_t frame = fast_rand( health ) % 4;
}

void g( void )
{
	ASSERT ( fast_rand ( 1 ) == 2 );
}

uint16_t fast_rand( int x ) {
	return( x + 1 );
}
#endif

void
testBug( void ) {
#if 0 // Bug not yet fixed
	ASSERT ( fast_rand ( 1 ) == 2 );
	g ();
#endif
}

