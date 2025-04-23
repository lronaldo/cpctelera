/* bug-3523.c
   An issue in handling register arguments of asm functions with __preserves_regs specification.
 */

#include <testfwk.h>

#pragma disable_warning 85

typedef struct tSpriteStruct {
  char dummy;
  signed char handle;
} SpriteStruct;

SpriteStruct sprites[2];

#if (defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_z80n)) && __SDCCCALL == 1
signed char SMS_addSprite (unsigned int y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
__asm
    ld a,l
    ret
__endasm;
}
#elif defined(__SDCC_sm83) && __SDCCCALL == 1
signed char SMS_addSprite (unsigned int y, unsigned int x_tile) __naked __preserves_regs(d,e,iyh,iyl) __sdcccall(1) {
__asm
    ld a,e
    ret
__endasm;
}
#else
signed char SMS_addSprite (unsigned int y, unsigned int x_tile) {
	return y;
}
#endif

void f(void) {
  sprites[0].handle = SMS_addSprite(0, 0);
  sprites[1].handle = SMS_addSprite(0, 0);
}

void
testBug (void) {
	sprites[0].handle = -1;
	sprites[1].handle = -1;
	f();
	ASSERT (sprites[0].handle == 0);
	ASSERT (sprites[1].handle == 0);
}

