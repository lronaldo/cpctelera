/* bug-3101.c
   A bug in gbz80 code generation when storing a value in l or h onto the stack.
 */

#include <testfwk.h>

#include <stdio.h>
#include <string.h>

typedef unsigned char UBYTE;
typedef unsigned int UWORD;
typedef int WORD;

typedef struct {
    WORD x, y;
} fly_coord_t; 

fly_coord_t flies[3]; // = {{(14 * 8), (5 * 8)}, {(6 * 8), (9 * 8)}, {(22 * 8), (12 * 8)}};
fly_coord_t * fly_ptr;

UBYTE __temp_i, __temp_j, __temp_k, __temp_l, __temp_m; 

int my_puts(const char *s)
{
    ASSERT (!strcmp(s, "ok"));
}

fly_coord_t * find_firefly(UBYTE tile_x, UBYTE tile_y) {
    UBYTE ff_tile_x, ff_tile_y;
    for (__temp_i = 0; __temp_i < 3; __temp_i++) {
        fly_ptr = &flies[__temp_i];
        
        ff_tile_x = fly_ptr->x >> 3, ff_tile_y = fly_ptr->y >> 3;        
        
        if ((ff_tile_x >= tile_x) && (ff_tile_x <= tile_x + 2) &&
            (ff_tile_y >= tile_y) && (ff_tile_y <= tile_y + 1)) return fly_ptr;
    }
    return 0;
}

UBYTE dizzy_catches_firefly(UBYTE tile_x, UBYTE tile_y, UBYTE id) {
    id;
    fly_coord_t * temp_fly;
    temp_fly = find_firefly(tile_x, tile_y);
    if (temp_fly) my_puts("ok"); else my_puts("fail");
    return 0;
}


void
testBug(void)
{
    flies[0].x=7*8;    flies[0].y=16*8+5;
    flies[1].x=11*8+2; flies[1].y=14*8+3;
    flies[2].x=18*8;   flies[2].y=5*8+1;
    dizzy_catches_firefly(10, 14, 0);
}

