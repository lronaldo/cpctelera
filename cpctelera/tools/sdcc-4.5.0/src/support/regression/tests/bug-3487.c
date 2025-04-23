/* bug-3481.c
   An issue in declarations that resulted in variables on the stack overwriting each other
 */

#include <testfwk.h>

#pragma disable_warning 85

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later

#include <stdint.h>
#include <stdio.h>

// We need a function definition before the affected function to trigger the bug.
void somedummyfunction(void)
{
}

// point
typedef struct {

    int16_t x;
    int16_t y;

} maths_point;

void TextNumber( uint8_t x, uint8_t y, uint16_t value );

void EnemyRender(void) {

    maths_point plane; // This declaration was handled wrongly, resulting in it being later put into the same location as sprite below.

    int16_t posX = 2208; // This one was also handled wrongly initially, but optimizations later calculated the correct live-range
    int16_t posY = 3680; // This one was also handled wrongly initially, but optimizations later calculated the correct live-range

    plane.x = 0;
    plane.y = 63;

    uint8_t y = 16;

    {
        int16_t enemyX = 117 * 16;
        int16_t enemyY = 250 * 16;

        maths_point sprite;
        sprite.x = enemyX - posX;
        sprite.y = enemyY - posY;

        int16_t planeYspriteX = plane.y * sprite.x;
        int16_t planeXspriteY = plane.x * sprite.y;
        int16_t planeSpriteDiff = planeXspriteY - planeYspriteX;
        int16_t transformY = planeSpriteDiff / 64; // this is actually the depth inside the screen, that what Z is in 3D

        TextNumber( 0,16,transformY);
    }
}

void TextNumber( uint8_t x, uint8_t y, uint16_t value )
{
	ASSERT( value == 330 );
}

void
testBug (void)
{
	EnemyRender ();
}

