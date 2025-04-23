/* bug-3523.c
   Variables on the stack were incorrectly allocated overlapping.
 */

#include <testfwk.h>

#pragma disable_warning 85

// Based on code by "Under4Mhz" licensed under GPL 2.0 or later

#include <stdint.h>
#include <stdbool.h>

typedef struct {

    int x;
    int y;

} maths_point;

void PlayerGridNextGet( maths_point *next ) {

    next->x = 1;
    next->y = 2;
}

void PlayerGridGet( maths_point *current ) {

    current->x = 3;
    current->y = 4;
}

int MapGet( int x, int y ) { return 0; }

int StateDoorHiddenOpenSet( int x, int y ) { ASSERT (x == 1); ASSERT (y == 2);} // Bug visible via x == 3 && y == 4 here.

bool ObjectIsHiddenDoor( char ch ) { return true; }

void SoundDoorOpen() {} // Weirdly, this empty function definition is necesssary to trigger the bug.

void Action() {

    maths_point next;
    PlayerGridNextGet( &next );
    char ch = MapGet( next.x, next.y );

    maths_point current;
    PlayerGridGet( &current );
    char chCurrent = MapGet( current.x, current.y );

    // Hidden door open
    StateDoorHiddenOpenSet( next.x, next.y );
}

void
testBug (void) {
    Action();
}

