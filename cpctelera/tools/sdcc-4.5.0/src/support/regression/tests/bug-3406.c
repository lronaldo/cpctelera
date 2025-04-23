/* bug-3406.c
   A bug in z80 subtraction codegen attempting to use iyl as right operand even on argets where this is not possible (triggering an assettion in codegen).
 */
 
#include <testfwk.h>

#pragma disable_warning 85

#ifndef __SDCC_z80
#define __banked
#endif

// GPL Version 2.0
#include <stdint.h>
uint8_t KeyMenuWait();

///< Show product menu
uint16_t SelectAmount(uint16_t amount, uint8_t step, const char *post) __banked {

    char text[8];

    for( ;; ) {

        *text = 0;

        uint8_t key = KeyMenuWait();

        switch( key ) {

                // More
            case 0:
                if ( amount < 1000 ) amount += step;
                break;

                // Less
            case 1:
                if ( amount > step ) amount -= step;
                break;

            case 2:
            case 3:
                return 0; // cancel

            case 4:
            case 5:
                return amount;
        }
    }
}

void testBug(void) {
	ASSERT(SelectAmount(0, 1, 0) == 0);
}

uint8_t KeyMenuWait() {
	static uint8_t i;
	return i++;
}

#ifdef __SDCC_z80
void dummy (void) __naked
{
__asm
get_bank::
set_bank::
	ret
__endasm;
}
#endif

