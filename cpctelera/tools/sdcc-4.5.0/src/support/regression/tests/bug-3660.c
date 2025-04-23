/* bug-3660.c
   A segfault in operation width narrowing optimization.
*/

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later

#include <testfwk.h>

#include <stdint.h>

typedef struct {

    int8_t facing;
    uint8_t frame;

} Enemy;

void EnemyRender( Enemy *enemy ) {

    uint8_t direction = enemy->facing < 0 ? 1: 0;

    uint8_t frame = enemy->frame / 2;

    uint8_t frameDirection = direction ? 1 : 0;
}

void
testBug( void )
{
	Enemy e;
	e.facing = 0;
	e.frame = 0;
	EnemyRender( &e );
}

