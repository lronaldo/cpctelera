/* bug-3459.c
   Multiple codegen issues related to bit-fields and pointers to pointers to structs containing them.
 */

#include <testfwk.h>

/// Test derived from code by SF user "Under4Mhz" in 2022.
 
// GPL Version 2.0
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct { unsigned char x; unsigned char y; } maths_point_u8;
typedef struct { int x; int y; } maths_point;

typedef struct {

    maths_point_u8 position;
    unsigned char direction : 4;
    unsigned char type : 4;

} ObjectData;

typedef struct {

    const ObjectData *objects;
    unsigned int objectCount;
    const char * const *map;
    const ObjectData *start;

} LevelData;

const ObjectData level1Start = { .direction = 1, .type = 1, .position = { 34, 57 } };

const LevelData level1Data = { .objects = 0, .objectCount = 37, .map = 0, .start = &level1Start };

const LevelData *levelData = &level1Data;

void MapStart( maths_point *position, unsigned char *direction ) {

    position->x = levelData->start->position.x + 1;
    position->y = levelData->start->position.y;
    *direction = levelData->start->direction; // The write to *direction was generated as if it was a write to a bit-field.
}

void testBug( void ) {
    maths_point p;
    unsigned char direction;

    MapStart( &p, &direction );

	ASSERT (direction == 1);
}

