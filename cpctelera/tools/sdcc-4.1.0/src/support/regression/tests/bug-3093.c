/* bug-3093.c
   An optimization used an incorrect cast to bool.
 */

#include <testfwk.h>

#include <stdint.h>
#include <string.h>

#define UINT8 uint8_t
#define UINT16 uint16_t

#define U_LESS_THAN(A, B) ((A) - (B) & 0x8000u)
#define DISTANCE(A, B) (U_LESS_THAN(A, B) ? (B - A) : (A - B))

struct Sprite {
	UINT8 anim_accum_ticks;
	UINT8 anim_speed;
	UINT8 anim_frame;
	UINT8 frame;

	UINT16 x;
	UINT16 y;
};

struct Sprite spr, tgt;

struct Sprite *THIS, *scroll_target;

#define DIST_ACTION 10

int mputs(const char *s)
{
    ASSERT (!strcmp (s, "no action"));
}

void test() {
    if(U_LESS_THAN(DISTANCE(THIS->x + 8, scroll_target->x + 8), DIST_ACTION)) mputs("action"); else mputs("no action");
}

void
testBug(void){
    spr.x = 10, THIS = &spr;
    tgt.x = 30, scroll_target = &tgt;
    test();
    spr.x = 5;
    tgt.x = 5;    
    test();
}

