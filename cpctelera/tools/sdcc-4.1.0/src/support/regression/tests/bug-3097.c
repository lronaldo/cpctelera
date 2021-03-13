/* bug-3097.c
   gbz80 backend overwrote still-needed value by using hl as address register for a global variable when the to-be-stored value was allocated to h or l.
 */


#include <testfwk.h>

#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory

#define max_scene_x 9
#define max_scene_y 9
#define max_scene_z 4

#define to_x(x,y,z) ((x) + (y))
#define to_y(x,y,z) ((7u * 8u) + ((x) << 2u) - ((y) << 2u) - ((z) << 3u))
#define to_coords(x,y,z) (((x) << 8u) | ((8u - (y)) << 4u) | (z))
#define from_coords(coord, x, y, z) (x = ((coord) >> 8u), y = (8u - ((coord) >> 4u) & 0x0f), z = ((coord) & 0x0f))

typedef unsigned char UBYTE;
typedef unsigned int UWORD;

typedef unsigned char scene_t[max_scene_x][max_scene_z][max_scene_y];

typedef struct scene_item_t {
    UBYTE id, x, y, n;
    UWORD coords;
    struct scene_item_t * next;
} scene_item_t;

scene_t collision_buf;

void clear_map(void * data) {
    data;
}

void scene_to_map(const scene_item_t * sour, scene_t * dest) {
    static scene_item_t * src;
    static UBYTE x, y, z;
    
    clear_map(dest);
    
    src = (scene_item_t *)sour;
    while (src) {
        from_coords(src->coords, x, y, z);
        if ((x < max_scene_x) && (y < max_scene_y) && (z < max_scene_z)) {
            (*dest)[x][z][y] = src->id + 1;
        }
        src = src->next;
    }    
}
const scene_item_t scene_items[] = {
{.id=1, .x=to_x(0, 8, 0), .y=to_y(0, 8, 0), .coords=to_coords(0, 8, 0), .next=&scene_items[1]},
{.id=1, .x=to_x(0, 8, 1), .y=to_y(0, 8, 1), .coords=to_coords(0, 8, 1), .next=0},
};

#endif

void
testBug(void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
    scene_to_map(scene_items, &collision_buf);
    
    ASSERT(collision_buf[0][0][max_scene_y - 1] == 2);
    ASSERT(collision_buf[0][1][max_scene_y - 1] == 2);
#endif
}

